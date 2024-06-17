/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include "Common/Config.h"
#ifdef MEDIAINFO_DLL
    #include "MediaInfoDLL/MediaInfoDLL.h"
    #define MediaInfoNameSpace MediaInfoDLL
#elif defined MEDIAINFO_STATIC
    #include "MediaInfoDLL/MediaInfoDLL_Static.h"
    #define MediaInfoNameSpace MediaInfoDLL
#else
    #include "MediaInfo/MediaInfoList.h"
    #define MediaInfoNameSpace MediaInfoLib
#endif
#include "Common/Merge.h"
#include "TimeCode.h"
#include "MediaInfo/MediaInfo_Events.h"
#include <chrono>
#include <vector>
using namespace MediaInfoNameSpace;
using namespace std;

#if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
#include "Common/ProcessFileWrapper.h"
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void UpdateDynamicDisplay(const string& Content = {});
string MediaInfo_Version();
//---------------------------------------------------------------------------

//***************************************************************************
// Enums
//***************************************************************************

#if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
enum rewind_mode {
    Rewind_Mode_None,
    Rewind_Mode_TimeCode,
    Rewind_Mode_TimeCode2,
    Rewind_Mode_Abst,
};

enum capture_mode {
    Capture_Mode_DV,
#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
    Capture_Mode_DeckLink,
#endif
};
#endif

#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
enum decklink_video_mode {
    Decklink_Video_Mode_NTSC,
    Decklink_Video_Mode_PAL,
    Decklink_Video_Mode_Max
};

enum decklink_video_source {
    Decklink_Video_Source_SDI,
    Decklink_Video_Source_HDMI,
    Decklink_Video_Source_Optical,
    Decklink_Video_Source_Component,
    Decklink_Video_Source_Composite,
    Decklink_Video_Source_SVideo,
    Decklink_Video_Source_Max
};

enum decklink_audio_source {
    Decklink_Audio_Source_Embedded,
    Decklink_Audio_Source_AESEBU,
    Decklink_Audio_Source_Analog,
    Decklink_Audio_Source_AnalogXLR,
    Decklink_Audio_Source_AnalogRCA,
    Decklink_Audio_Source_Microphone,
    Decklink_Audio_Source_Max
};

enum decklink_pixel_format {
    Decklink_Pixel_Format_Unspecified,
    Decklink_Pixel_Format_8BitYUV,
    Decklink_Pixel_Format_10BitYUV,
    Decklink_Pixel_Format_8BitARGB,
    Decklink_Pixel_Format_8BitBGRA,
    Decklink_Pixel_Format_10BitRGB,
    Decklink_Pixel_Format_Max
};

enum decklink_timecode_format {
    Decklink_Timecode_Format_RP188_VITC,
    Decklink_Timecode_Format_RP188_VITC2,
    Decklink_Timecode_Format_RP188_LTC,
    Decklink_Timecode_Format_RP188_HFR,
    Decklink_Timecode_Format_RP188_ANY,
    Decklink_Timecode_Format_VITC,
    Decklink_Timecode_Format_VITC2,
    Decklink_Timecode_Format_Serial,
    Decklink_Timecode_Format_Max
};
#endif

//***************************************************************************
// Class file
//***************************************************************************

class file
{
public:
    MediaInfo MI;
    std::vector<MediaInfo_Event_DvDif_Change_0*> PerChange;
    std::vector<MediaInfo_Event_DvDif_Analysis_Frame_1*> PerFrame;
    struct caption_fielddata
    {
        caption_fielddata(const MediaInfo_int8u* NewData)
        {
            Data[0] = NewData[0];
            Data[1] = NewData[1];
        }

        int compare(const MediaInfo_int8u* NewData)
        {
            if (Data[0] < NewData[0])
                return -1;
            if (Data[0] > NewData[0])
                return 1;
            if (Data[1] < NewData[1])
                return -1;
            if (Data[1] > NewData[1])
                return 1;
            return 0;
        }

        uint8_t Data[2];
    };
    struct captions_fielddata
    {
        captions_fielddata(double PTS, double DUR, MediaInfo_int64u NewStartFrameNumber) :
            PTS(PTS),
            DUR(DUR),
            StartFrameNumber(NewStartFrameNumber)
        {}
        double PTS;
        double DUR;
        MediaInfo_int64u StartFrameNumber;
        vector<caption_fielddata> Captions;
    };
    struct captions_data
    {
        vector<captions_fielddata> FieldData[2];
    };
    vector<captions_data> PerFrame_Captions_PerSeq_PerField;
    size_t FrameNumber = 0;
    double FrameRate = 0;

    file();
    ~file();

    return_value Parse(const String& FileName);
    void Parse_Buffer(const uint8_t* Buffer, size_t Buffer_Size);
    void Terminate();

    bool TransportControlsSupported();
    #if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
    void RewindToTimeCode(TimeCode TC);
    void RewindToAbst(int Abst);
    #endif
    void AddChange(const MediaInfo_Event_DvDif_Change_0* FrameData);
    void AddFrameAnalysis(const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData);
    void AddFrameData(const MediaInfo_Event_Global_Demux_4* FrameData);

    // Merge
    void Merge_Finish() { Merge.Finish(); }

private:
    size_t Merge_FilePos;
    dv_merge Merge;
    bool no_sourceorcontrol_aud_set_in_first_frame = false;

    #if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
public:
    ControllerBaseWrapper* Controller = nullptr;
    BaseWrapper* Capture = nullptr;
    FileWrapper* Wrapper = nullptr;
    float Speed_Before = 0;
    float Speed_After = 0;
    bool PauseRequested = false;
    rewind_mode RewindMode = Rewind_Mode_None;
    TimeCode RewindTo_TC;
    int RewindTo_Abst = 0;
    int DelayedPlay = 0;
    bool TerminateRequested = false;
    bool TimeOutReached = false;
    capture_mode CaptureMode = Capture_Mode_DV;
    #endif
};
