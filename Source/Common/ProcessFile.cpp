/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/ProcessFile.h"
#include "ZenLib/Ztring.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

//***************************************************************************
// Callback
//***************************************************************************

void __stdcall Event_CallBackFunction(unsigned char* Data_Content, size_t Data_Size, void* UserHandler_Void)
{
    //Retrieving UserHandler
    file*                               UserHandler = (file*)UserHandler_Void;
    struct MediaInfo_Event_Generic*     Event_Generic = (struct MediaInfo_Event_Generic*) Data_Content;
    unsigned char                       ParserID;
    unsigned short                      EventID;
    unsigned char                       EventVersion;

    //Integrity test
    if (Data_Size < 4)
        return; //There is a problem

    //Retrieving EventID
    ParserID = (unsigned char)((Event_Generic->EventCode & 0xFF000000) >> 24);
    EventID = (unsigned short)((Event_Generic->EventCode & 0x00FFFF00) >> 8);
    EventVersion = (unsigned char)(Event_Generic->EventCode & 0x000000FF);
    switch (ParserID)
    {
    case MediaInfo_Parser_DvDif:
        switch (EventID)
        {
        case MediaInfo_Event_DvDif_Analysis_Frame: if (EventVersion == 1 && Data_Size >= sizeof(struct MediaInfo_Event_DvDif_Analysis_Frame_1)) UserHandler->AddFrame((MediaInfo_Event_DvDif_Analysis_Frame_1*)Event_Generic); break;
        case MediaInfo_Event_DvDif_Change: if (EventVersion == 0 && Data_Size >= sizeof(struct MediaInfo_Event_DvDif_Change_0)) UserHandler->AddChange((MediaInfo_Event_DvDif_Change_0*)Event_Generic); break;
        }
        break;
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
file::file(const String& FileName)
{
    FrameNumber = 0;

    MI.Option(__T("File_Event_CallBackFunction"), __T("CallBack=memory://") + Ztring::ToZtring((size_t)&Event_CallBackFunction) + __T(";UserHandler=memory://") + Ztring::ToZtring((size_t)this));
    MI.Option(__T("File_DvDif_Analysis"), __T("1"));
    MI.Open(FileName);

    // Filing some info
    FrameRate = Ztring(MI.Get(Stream_Video, 0, __T("FrameRate_Original"))).To_float64();
    if (!FrameRate)
        FrameRate = Ztring(MI.Get(Stream_Video, 0, __T("FrameRate"))).To_float64();
    if (!FrameRate || (FrameRate >= 29.97 && FrameRate <= 29.98))
        FrameRate = double(30 / 1.001); // Default if no frame rate available, or better rounding
}

//---------------------------------------------------------------------------
file::~file()
{
    for (auto& Frame : PerFrame)
    {
        delete[] Frame->Errors;
        delete[] Frame->Video_STA_Errors;
        delete[] Frame->Audio_Data_Errors;
        delete Frame;
    }
    for (auto& Change : PerChange)
    {
        delete Change;
    }
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void file::AddChange(const MediaInfo_Event_DvDif_Change_0* FrameData)
{
    FrameNumber++; // Event FrameCount is currently wrong

    // Check if there is a change we support
    if (!PerChange.empty())
    {
        const auto Current = PerChange.back();
        if (Current->Width == FrameData->Width
            && Current->Height == FrameData->Height
            && Current->VideoChromaSubsampling == FrameData->VideoChromaSubsampling
            && Current->VideoScanType == FrameData->VideoScanType
            && Current->VideoRatio_N == FrameData->VideoRatio_N
            && Current->VideoRatio_D == FrameData->VideoRatio_D
            && Current->VideoRate_N == FrameData->VideoRate_N
            && Current->VideoRate_D == FrameData->VideoRate_D
            && Current->AudioRate_N == FrameData->AudioRate_N
            && Current->AudioRate_D == FrameData->AudioRate_D
            && Current->AudioChannels == FrameData->AudioChannels
            && Current->AudioBitDepth == FrameData->AudioBitDepth)
        {
            return;
        }
    }

    MediaInfo_Event_DvDif_Change_0* ToPush = new MediaInfo_Event_DvDif_Change_0();
    std::memcpy(ToPush, FrameData, sizeof(MediaInfo_Event_DvDif_Change_0));
    ToPush->FrameNumber = FrameNumber - 1; // Event FrameCount is currently wrong
    PerChange.push_back(ToPush);
}

//---------------------------------------------------------------------------
void file::AddFrame(const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData)
{
    MediaInfo_Event_DvDif_Analysis_Frame_1* ToPush = new MediaInfo_Event_DvDif_Analysis_Frame_1();
    std::memcpy(ToPush, FrameData, sizeof(MediaInfo_Event_DvDif_Analysis_Frame_1));
    if (FrameData->Errors)
    {
        size_t SizeToCopy = std::strlen(FrameData->Errors) + 1;
        auto Errors = new char[SizeToCopy];
        std::memcpy(Errors, FrameData->Errors, SizeToCopy);
        ToPush->Errors = Errors;
    }
    if (FrameData->Video_STA_Errors)
    {
        size_t SizeToCopy = FrameData->Video_STA_Errors_Count * sizeof(size_t);
        auto Video_STA_Errors = new size_t[SizeToCopy];
        std::memcpy(Video_STA_Errors, FrameData->Video_STA_Errors, SizeToCopy);
        ToPush->Video_STA_Errors = Video_STA_Errors;
    }
    if (FrameData->Audio_Data_Errors)
    {
        size_t SizeToCopy = FrameData->Audio_Data_Errors_Count * sizeof(size_t);
        auto Audio_Data_Errors = new size_t[SizeToCopy];
        std::memcpy(Audio_Data_Errors, FrameData->Audio_Data_Errors, SizeToCopy);
        ToPush->Audio_Data_Errors = Audio_Data_Errors;
    }
    PerFrame.push_back(ToPush);
}
