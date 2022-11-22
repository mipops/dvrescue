/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Output.h"
#include "ZenLib/Ztring.h"
#include <cfloat>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <thread>
using namespace ZenLib;
using namespace std;
using namespace std::chrono;
#include "Common/ProcessFile.h"
#ifdef ENABLE_AVFCTL
#include "Common/AvfCtlWrapper.h"
#endif
#ifdef ENABLE_SIMULATOR
#include "Common/SimulatorWrapper.h"
#endif
#if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
FileWrapper* Wrapper = nullptr;
#endif
bool InControl = false;
uint64_t Device_Pos = (uint64_t)-1;
char Device_Command = 0;
bool Device_ForceCapture = false;
unsigned int Device_Mode = (unsigned int)-1;
float Device_Speed = FLT_MIN;
auto Device_LastPacketTime = chrono::steady_clock::now();
uint64_t VariableSize(const uint8_t* Buffer, size_t& Buffer_Offset, size_t Buffer_Size);
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
        case MediaInfo_Event_DvDif_Analysis_Frame: if (EventVersion == 1 && Data_Size >= sizeof(struct MediaInfo_Event_DvDif_Analysis_Frame_1)) UserHandler->AddFrameAnalysis((MediaInfo_Event_DvDif_Analysis_Frame_1*)Event_Generic); break;
        case MediaInfo_Event_DvDif_Change: if (EventVersion == 0 && Data_Size >= sizeof(struct MediaInfo_Event_DvDif_Change_0)) UserHandler->AddChange((MediaInfo_Event_DvDif_Change_0*)Event_Generic); break;
        case MediaInfo_Event_Global_Demux: if (EventVersion == 4 && Data_Size >= sizeof(struct MediaInfo_Event_Global_Demux_4)) UserHandler->AddFrameData((MediaInfo_Event_Global_Demux_4*)Event_Generic); break;
        }
        break;
    case MediaInfo_Parser_Global:
        switch (EventID)
        {
        case MediaInfo_Event_Global_Demux: if (EventVersion == 4 && Data_Size >= sizeof(struct MediaInfo_Event_Global_Demux_4)) UserHandler->AddFrameData((MediaInfo_Event_Global_Demux_4*)Event_Generic); break;
        }
        break;
    }
}

//***************************************************************************
// Control
//***************************************************************************

#if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
void InputControl_Char(file* F, char C)
{
    switch (C)
    {
    case 'R': F->PauseRequested = false; F->Controller->SetPlaybackMode(Playback_Mode_NotPlaying, -2.0); break;
    case 'r': F->PauseRequested = false; F->Controller->SetPlaybackMode(Playback_Mode_Playing   , -1.0); break;
    case 'q': F->PauseRequested = false; F->Controller->SetPlaybackMode(Playback_Mode_NotPlaying,  0.0); break;
    case 's': F->PauseRequested = true ; F->Controller->SetPlaybackMode(Playback_Mode_NotPlaying,  0.0); break;
    case 'f': F->PauseRequested = false; F->Controller->SetPlaybackMode(Playback_Mode_Playing   ,  1.0); break;
    case 'F': F->PauseRequested = false; F->Controller->SetPlaybackMode(Playback_Mode_NotPlaying,  2.0); break;
    default: return;
    }
}
void InputControl (file* F)
{
    for (;;)
    {
        auto C = getc(stdin);
        InputControl_Char(F, C);
        if (C == 'q')
            return;

        this_thread::yield();
    }
}
#endif

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
size_t Merge_FilePos_Total = 0;
file::file()
{
    Merge_FilePos = Merge_FilePos_Total++;
}

//---------------------------------------------------------------------------
void file::Parse(const String& FileName)
{
    MI.Option(__T("File_Event_CallBackFunction"), __T("CallBack=memory://") + Ztring::ToZtring((size_t)&Event_CallBackFunction) + __T(";UserHandler=memory://") + Ztring::ToZtring((size_t)this));
    MI.Option(__T("File_DvDif_Analysis"), __T("1"));
    if (!Merge_InputFileNames.empty() && (Merge_InputFileNames.front() == "-" || TransportControlsSupported())) // Only if from stdin (not supported in other cases)
        MI.Option(__T("File_Demux_Unpacketize"), __T("1"));
    if (!Merge_InputFileNames.empty() && TransportControlsSupported())
        MI.Option(__T("File_FrameIsAlwaysComplete"), __T("1"));

    if (Verbosity == 10)
        cerr << "Debug: opening (in) \"" << Ztring(FileName).To_Local() << "\"..." << endl;
    #if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
    if (Device_Command == 1)
    {
        size_t i = 0;
        #ifdef ENABLE_SIMULATOR
            for (;;)
            {
                auto Name = SimulatorWrapper::GetDeviceName(i);
                if (Name.empty())
                    break;
                cout << Name << '\n';
                i++;
            }
        #endif
        #ifdef ENABLE_AVFCTL
            for (;;)
            {
                auto Name = AVFCtlWrapper::GetDeviceName(i);
                if (Name.empty())
                    break;
                cout << Name << '\n';
                i++;
            }
        #endif
        return;
    }
    Controller = nullptr;
    if (Device_Pos == (size_t)-1 && FileName.rfind(__T("device://"), 0) == 0)
        Device_Pos = (size_t)Ztring(FileName.substr(9)).To_int64u();
    if (Device_Pos == (size_t)-1 && Device_Command)
        Device_Pos = 0;
    if (Device_Pos != (size_t)-1)
    {
        if (false)
            ;
        #ifdef ENABLE_SIMULATOR
            else if (Device_Pos < SimulatorWrapper::GetDeviceCount())
                Controller = new SimulatorWrapper(Device_Pos);
        #endif
        #ifdef ENABLE_AVFCTL
            else if (Device_Pos < AVFCtlWrapper::GetDeviceCount())
                Controller = new AVFCtlWrapper(Device_Pos);
        #endif
    }
    if (Controller)
    {
        if (Device_Command && !Device_ForceCapture)
        {
            if (Device_Command == 2)
            {
                FileWrapper Wrapper(this);
                Controller->CreateCaptureSession(&Wrapper);
                this_thread::sleep_for(chrono::milliseconds(500)); // give time for the driver to retrieves status from the device
                auto Status = Controller->GetStatus();
                Controller->StopCaptureSession();
                cout << Status << '\n';
            }
            if (Device_Command == 3)
            {
                Controller->SetPlaybackMode((playback_mode)Device_Mode, Device_Speed);
            }
            if (Device_Command >= 0x20)
            {
                InputControl_Char(this, Device_Command);
            }
            return;
        }
        Speed_Before = Controller->GetSpeed();
        auto InputHelper = InControl ? new thread(InputControl, this) : nullptr;
        FileWrapper Wrapper(this);
        MI.Open_Buffer_Init();
        Controller->CreateCaptureSession(&Wrapper);
        for (;;)
        {
            Controller->StartCaptureSession();
            if (!Device_Command)
                Controller->SetPlaybackMode(Playback_Mode_Playing, 1.0);
            else if (Device_Command != -1)
            {
                if (Device_Command == 3)
                    Controller->SetPlaybackMode((playback_mode)Device_Mode, Device_Speed);
                else if (Device_Command >= 0x20)
                    InputControl_Char(this, Device_Command);
                Device_Command = -1;
            }
            Controller->WaitForSessionEnd();
            Controller->StopCaptureSession();
            if (!InputHelper)
                break;
            if (!PauseRequested) // If stop was not requested, it is end of stream
            {
                InputHelper->join();
                delete InputHelper;
                break;
            }
            while (PauseRequested)
                this_thread::yield();
        }
        MI.Open_Buffer_Finalize();
    }
    else
    #endif
    {
        MI.Open(FileName);
    }
    if (Verbosity == 10)
        cerr << "Debug: opening (in) \"" << Ztring(FileName).To_Local() << "\"... Done." << endl;

    // Filing some info
    FrameRate = Ztring(MI.Get(Stream_Video, 0, __T("FrameRate_Original"))).To_float64();
    if (!FrameRate)
        FrameRate = Ztring(MI.Get(Stream_Video, 0, __T("FrameRate"))).To_float64();
    if (!FrameRate || (FrameRate >= 29.97 && FrameRate <= 29.98))
        FrameRate = double(30 / 1.001); // Default if no frame rate available, or better rounding
}

//---------------------------------------------------------------------------
void file::Parse_Buffer(const uint8_t* Buffer, size_t Buffer_Size)
{
    MI.Open_Buffer_Continue(Buffer, Buffer_Size);
}

//---------------------------------------------------------------------------
file::~file()
{
    #if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
    if (Controller)
        delete Controller;
    #endif

    for (auto& Frame : PerFrame)
    {
        delete[] Frame->Errors;
        delete[] Frame->Video_STA_Errors;
        delete[] Frame->Audio_Data_Errors;
        delete[] Frame->MoreData;
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
bool file::TransportControlsSupported()
{
#if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
    if (!Merge_InputFileNames[0].find("device://"))
        return true;
#endif
#ifdef ENABLE_SIMULATOR
    if (!Merge_InputFileNames[0].find("simulator://"))
        return true;
#endif
    return false;
}

//---------------------------------------------------------------------------
#if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
void file::RewindToTimeCode(TimeCode TC)
{
    RewindMode=Rewind_Mode_TimeCode;
    RewindTo_TC=TC;
    Controller->SetPlaybackMode(Playback_Mode_Playing, -1.0);
}
#endif

//---------------------------------------------------------------------------
#if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
void file::RewindToAbst(int Abst)
{
    RewindMode=Rewind_Mode_Abst;
    RewindTo_Abst=Abst;
    Controller->SetPlaybackMode(Playback_Mode_Playing, -1.0);
}
#endif

//---------------------------------------------------------------------------
void file::AddChange(const MediaInfo_Event_DvDif_Change_0* FrameData)
{
    #if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
    if (RewindMode!=Rewind_Mode_None)
        return;
    #endif

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
            && Current->AudioBitDepth == FrameData->AudioBitDepth
            && Current->Captions_Flags == FrameData->Captions_Flags)
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
void file::AddFrameAnalysis(const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData)
{
    #if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
    abst_bf AbstBf_Temp(FrameData->AbstBf);
    if (DelayedPlay)
    {
        timecode TC_Temp(FrameData);
        if (TC_Temp.HasValue())
        {
            TimeCode TC(TC_Temp.TimeInSeconds() / 3600, (TC_Temp.TimeInSeconds() / 60) % 60, TC_Temp.TimeInSeconds() % 60, TC_Temp.Frames(), 30 /*TEMP*/, TC_Temp.DropFrame());
            if (Verbosity == 10)
            {
                cerr << "Rewind ";
                string AbstString = to_string(AbstBf_Temp.AbsoluteTrackNumber());
                if (AbstString.size() < 6)
                    AbstString.insert(0, 6 - AbstString.size(), ' ');
                cerr << AbstString;
                cerr << ' ';
                cerr << TC.ToString();
                cerr << setw(Merge_Rewind_Count + 5) << ' ';
                cerr << to_string(GetDvSpeed(*FrameData));
                cerr << '\n';
            }
        }
        DelayedPlay--;
        if (!DelayedPlay)
        {
            RewindMode = Rewind_Mode_TimeCode2;
            Controller->SetPlaybackMode(Playback_Mode_Playing, 1.0);
        }
        return;
    }
    if (RewindMode==Rewind_Mode_TimeCode || RewindMode==Rewind_Mode_TimeCode2)
    {
        timecode TC_Temp(FrameData);
        if (TC_Temp.HasValue())
        {
            TimeCode TC(TC_Temp.TimeInSeconds() / 3600, (TC_Temp.TimeInSeconds() / 60) % 60, TC_Temp.TimeInSeconds() % 60, TC_Temp.Frames(), 30 /*TEMP*/, TC_Temp.DropFrame());
            if (RewindMode==Rewind_Mode_TimeCode && TC.ToFrames()<RewindTo_TC.ToFrames())
            {
                if (Verbosity == 10)
                {
                    cerr << "Rewind ";
                    string AbstString = to_string(AbstBf_Temp.AbsoluteTrackNumber());
                    if (AbstString.size() < 6)
                        AbstString.insert(0, 6 - AbstString.size(), ' ');
                    cerr << AbstString;
                    cerr << ' ';
                    cerr << TC.ToString();
                    cerr << setw(Merge_Rewind_Count + 5) << ' ';
                    cerr << to_string(GetDvSpeed(*FrameData));
                    cerr << '\n';
                }
                DelayedPlay = 4;
                if (!DelayedPlay)
                {
                    RewindMode = Rewind_Mode_TimeCode2;
                    Controller->SetPlaybackMode(Playback_Mode_Playing, 1.0);
                }
                return; //Continue in forward mode
            }
            else if (RewindMode==Rewind_Mode_TimeCode2 && TC.ToFrames()>=RewindTo_TC.ToFrames()-1) // TEMP: do not do minus 1
            {
                if (Verbosity == 10)
                {
                    cerr << "Rewind ";
                    string AbstString = to_string(AbstBf_Temp.AbsoluteTrackNumber());
                    if (AbstString.size() < 6)
                        AbstString.insert(0, 6 - AbstString.size(), ' ');
                    cerr << AbstString;
                    cerr << ' ';
                    cerr << TC.ToString();
                    cerr << setw(Merge_Rewind_Count + 5) << ' ';
                    cerr << to_string(GetDvSpeed(*FrameData));
                    cerr << '\n';
                }
                RewindTo_TC = TimeCode();
                RewindMode = Rewind_Mode_None;
                return; //Last one
            }
            else
            {
                if (Verbosity == 10)
                {
                    cerr << "Rewind ";
                    string AbstString = to_string(AbstBf_Temp.AbsoluteTrackNumber());
                    if (AbstString.size() < 6)
                        AbstString.insert(0, 6 - AbstString.size(), ' ');
                    cerr << AbstString;
                    cerr << ' ';
                    cerr << TC.ToString();
                    cerr << setw(Merge_Rewind_Count + 5) << ' ';
                    cerr << to_string(GetDvSpeed(*FrameData));
                    cerr << '\n';
                }
                return; //Continue in rewind mode
            }
        }
        else
            return; //Continue in rewind mode
    }
    else if (RewindMode==Rewind_Mode_Abst)
    {
        abst_bf AbstBf_Temp(FrameData->AbstBf);
        if (AbstBf_Temp.HasAbsoluteTrackNumberValue())
        {
            if (AbstBf_Temp.AbsoluteTrackNumber()<=RewindTo_Abst)
            {
                RewindMode=Rewind_Mode_None;
                Controller->SetPlaybackMode(Playback_Mode_Playing, 1.0);
            }
            else
                return; //Continue in rewind mode
        }
        else
            return; //Continue in rewind mode
    }
    else if (RewindTo_TC.HasValue())
    {
        timecode TC_Temp(FrameData);
        if (TC_Temp.HasValue())
        {
            TimeCode TC(TC_Temp.TimeInSeconds() / 3600, (TC_Temp.TimeInSeconds() / 60) % 60, TC_Temp.TimeInSeconds() % 60, TC_Temp.Frames(), 30 /*TEMP*/, TC_Temp.DropFrame());
            if (TC.ToFrames()<RewindTo_TC.ToFrames())
                return;
            RewindTo_TC = TimeCode();
        }
        else
            return; //Continue in rewind mode
    }
    #endif
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
    if (FrameData->MoreData)
    {
        size_t SizeToCopy = *((size_t*)FrameData->MoreData) + sizeof(size_t);
        auto MoreData = new uint8_t[SizeToCopy];
        std::memcpy(MoreData, FrameData->MoreData, SizeToCopy);
        ToPush->MoreData = MoreData;
    }
    PerFrame.push_back(ToPush);

    coherency_flags Coherency(FrameData->Coherency_Flags);
    if (!no_sourceorcontrol_aud_set_in_first_frame && !(Coherency.no_pack_aud() || !Coherency.no_sourceorcontrol_aud()))
    {
        if (!PerChange.empty() && ToPush->FrameNumber == PerChange.back()->FrameNumber)
            no_sourceorcontrol_aud_set_in_first_frame = true;
    }
    if (no_sourceorcontrol_aud_set_in_first_frame && (Coherency.no_pack_aud() || !Coherency.no_sourceorcontrol_aud()) || GetDvSpeedHasChanged(PerFrame))
    {
        if (PerChange.back()->FrameNumber != FrameNumber - 1)
        {
            auto& PerChange_Last = PerChange.back();
            MediaInfo_Event_DvDif_Change_0* PerChange_ToPush = new MediaInfo_Event_DvDif_Change_0();
            std::memcpy(PerChange_ToPush, PerChange_Last, sizeof(MediaInfo_Event_DvDif_Change_0));
            PerChange_Last->AudioRate_N = 0;
            PerChange_Last->AudioRate_D = 0;
            PerChange_Last->AudioChannels = 0;
            PerChange_Last->AudioBitDepth = 0;
            PerChange_ToPush->FrameNumber = FrameNumber - 1; // Event FrameCount is currently wrong
            PerChange_ToPush->PCR = ToPush->PCR;
            PerChange_ToPush->DTS = ToPush->DTS;
            PerChange_ToPush->PTS = ToPush->PTS;
            PerChange_ToPush->DUR = ToPush->DUR;
            PerChange.push_back(PerChange_ToPush);
        }
        no_sourceorcontrol_aud_set_in_first_frame = false;
    }

    if (Merge_Out)
    {
        auto Speed = abs(Speed_Before) > abs(Speed_After) ? Speed_Before : Speed_After;
        Speed_Before = Speed_After;
        Merge.AddFrameAnalysis(Merge_FilePos, FrameData, Speed);
        #if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
        if (TransportControlsSupported() && Merge.RewindToTimeCode.HasValue())
        {
            RewindToTimeCode(Merge.RewindToTimeCode);
            Merge.RewindToTimeCode = TimeCode();
        }
        #endif
    }

    // Information
    if (!Merge_FilePos && Verbosity > 0 && Verbosity <= 7)
    {
        string Text;
        const int Formating_FrameCount_Width = 6;
        Text = to_string(FrameNumber);
        if (Text.size() < Formating_FrameCount_Width)
            Text.insert(0, Formating_FrameCount_Width - Text.size(), ' ');
        timecode TC_Temp(FrameData);
        if (TC_Temp.HasValue())
        {
            TimeCode TC(TC_Temp.TimeInSeconds() / 3600, (TC_Temp.TimeInSeconds() / 60) % 60, TC_Temp.TimeInSeconds() % 60, TC_Temp.Frames(), 30 /*TEMP*/, TC_Temp.DropFrame());
            Text += ' ';
            Text += TC.ToString();
        }
        rec_date_time RecDateTime(FrameData);
        string RecDateTime_String;
        if (RecDateTime.HasDate())
        {
            Text += ' ';
            date_to_string(Text, RecDateTime.Years(), RecDateTime.Months(), RecDateTime.Days());
        }
        if (RecDateTime.HasTime())
        {
            Text += ' ';
            timecode_to_string(Text, RecDateTime.TimeInSeconds(), TC_Temp.DropFrame(), RecDateTime.Frames());
        }
        if (Controller)
        {
            Text += ' ';
            Text += '(';
            Text += ::to_string(Speed_After);
            Text += 'x';
            auto DvSpeed = GetDvSpeed(*FrameData);
            if (DvSpeed != INT_MIN)
            {
                Text += ',';
                Text += ' ';
                Text += 'D';
                Text += 'V';
                Text += ' ';
                Text += std::to_string(DvSpeed);
            }
            Text += ')';
        }
        UpdateDynamicDisplay(Text);
    }
}

//---------------------------------------------------------------------------
void file::AddFrameData(const MediaInfo_Event_Global_Demux_4* FrameData)
{
    #if defined(ENABLE_AVFCTL) || defined(ENABLE_SIMULATOR)
    if (DelayedPlay)
        return;
    if (RewindMode!=Rewind_Mode_None)
        return;
    #endif

    // DV frame
    if (!FrameData->StreamIDs_Size || FrameData->StreamIDs[FrameData->StreamIDs_Size-1]==-1)
    {
        if (Merge_Out && (Merge_InputFileNames.empty() || Merge_InputFileNames[0] == "-" || Merge_InputFileNames[0].find("device://") == 0)) // Only for stdin
            Merge.AddFrameData(Merge_FilePos, FrameData->Content, FrameData->Content_Size);

        Device_LastPacketTime = chrono::steady_clock::now();

        return;
    }

    // Caption frame
    auto Dseq = FrameData->StreamIDs[FrameData->StreamIDs_Size-1];
    if ((Dseq & 0xFFFF00) != ((0x2 << 16) | (0x65 << 8))) // identifier with SCT = 0x2, PackType = 0x65, and Dseq
        return;
    Dseq &= 0xFF; // Extract Dseq
    if (Dseq >= 16)
        return;
    if (FrameData->Content_Size != 4)
        return;

    // Store
    for (int i = 0; i < 2; i++)
    {
        // Filter unneeded content
        if (FrameData->Content[i * 2] == 0x80 && FrameData->Content[i * 2 + 1] == 0x80)
            return; // Don't store empty content
        if (Dseq && PerFrame_Captions_PerSeq_PerField.size() == 1)
        {
            auto& PerSeq0 = PerFrame_Captions_PerSeq_PerField[0];
            auto& FieldData0 = PerSeq0.FieldData[i];
            if (!FieldData0.empty() && FieldData0.back().StartFrameNumber + FieldData0.back().Captions.size() - 1 == FrameNumber && !FieldData0.back().Captions.back().compare(FrameData->Content + i * 2))
                continue; // Content is same, not stored
        }

        // Store
        if (Dseq >= PerFrame_Captions_PerSeq_PerField.size())
            PerFrame_Captions_PerSeq_PerField.resize(Dseq + 1);
        auto& PerSeq = PerFrame_Captions_PerSeq_PerField[Dseq];
        PerSeq.PTS = FrameData->PTS / 1000000000.0;
        PerSeq.DUR = FrameData->DUR / 1000000000.0;
        auto& FieldData = PerSeq.FieldData[i];
        if (FieldData.empty() || FieldData.back().StartFrameNumber + FieldData.back().Captions.size() != FrameNumber)
            FieldData.emplace_back(FrameNumber);
        FieldData.back().Captions.emplace_back(FrameData->Content + i * 2);
    }
}

void UpdateDynamicDisplay(const string& Content)
{
    static size_t Content_Previous_Size = 0;
    string Temp(Content);
    if (Content_Previous_Size > Content.size())
        Temp.resize(Content_Previous_Size, ' ');
    Temp += '\r';
    cerr << Temp;
    Content_Previous_Size = Content.size();
}

string MediaInfo_Version()
{
    return Ztring(MediaInfo::Option_Static(__T("Info_Version"), String())).SubString(__T(" - v"), String()).To_UTF8();
}
