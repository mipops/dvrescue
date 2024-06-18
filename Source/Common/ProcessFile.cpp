/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Output.h"
#include "ZenLib/Ztring.h"
#include <functional>
#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cctype>
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
#ifdef ENABLE_LNX1394
#include "Common/LinuxWrapper.h"
#endif
#ifdef ENABLE_DECKLINK
#include "Common/DecklinkWrapper.h"
#endif
#ifdef ENABLE_SONY9PIN
#include "Common/Sony9PinWrapper.h"
#endif
#ifdef ENABLE_SIMULATOR
#include "Common/SimulatorWrapper.h"
#endif
#ifdef ENABLE_SONY9PIN
const char* Control_Port = nullptr;
#endif
#if defined (ENABLE_DECKLINK) || defined (ENABLE_SIMULATOR)
bool DeckLinkNativeControl = false;
uint8_t DeckLinkVideoMode = (uint8_t)Decklink_Video_Mode_NTSC;
uint8_t DeckLinkVideoSource = (uint8_t)Decklink_Video_Source_SDI;
uint8_t DeckLinkAudioSource = (uint8_t)Decklink_Audio_Source_Embedded;
uint8_t DeckLinkPixelFormat = (uint8_t)Decklink_Pixel_Format_Unspecified;
uint8_t DeckLinkTimecodeFormat = (uint8_t)Decklink_Timecode_Format_VITC;
#endif
bool InControl = false;
bool Foreground = false;
string Device = "";
char Device_Command = 0;
bool Device_ForceCapture = false;
unsigned int Device_Mode = (unsigned int)-1;
float Device_Speed = FLT_MIN;
auto Device_LastPacketTime = chrono::steady_clock::now();
uint64_t VariableSize(const uint8_t* Buffer, size_t& Buffer_Offset, size_t Buffer_Size);
//---------------------------------------------------------------------------

//***************************************************************************
// Helpers
//***************************************************************************
string JSON_Encode (const string& Data)
{
    string Result;
    for (string::size_type Pos=0; Pos<Data.size(); Pos++)
    {
        switch (Data[Pos])
        {
            case '\b': Result+="\\b"; break;
            case '\f': Result+="\\f"; break;
            case '\n': Result+="\\n"; break;
            case '\r': Result+="\\r"; break;
            case '\t': Result+="\\t"; break;
            case '"': Result+="\\\""; break;
            case '\\': Result+="\\\\"; break;
            default: Result+=Data[Pos];
        }
    }
     return Result;
}

//---------------------------------------------------------------------------
string Trim(string Data) {

    Data.erase(Data.begin(), std::find_if(Data.begin(), Data.end(), [](unsigned char c) {
        return !std::isspace(c);
    }));

    Data.erase(std::find_if(Data.rbegin(), Data.rend(), [](unsigned char c) {
        return !std::isspace(c);
    }).base(), Data.end());

    return Data;
}

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

#if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
void InputControl_Char(file* F, char C)
{
    switch (C)
    {
    case 'R': F->PauseRequested = false; F->Capture->SetPlaybackMode(Playback_Mode_NotPlaying, -2.0); break;
    case 'r': F->PauseRequested = false; F->Capture->SetPlaybackMode(Playback_Mode_Playing   , -1.0); break;
    case 'q': F->PauseRequested = false; F->Capture->SetPlaybackMode(Playback_Mode_NotPlaying,  0.0); break;
    case 's': F->PauseRequested = true ; F->Capture->SetPlaybackMode(Playback_Mode_NotPlaying,  0.0); break;
    case 'f': F->PauseRequested = false; F->Capture->SetPlaybackMode(Playback_Mode_Playing   ,  1.0); break;
    case 'F': F->PauseRequested = false; F->Capture->SetPlaybackMode(Playback_Mode_NotPlaying,  2.0); break;
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
return_value file::Parse(const String& FileName)
{
    MI.Option(__T("File_Event_CallBackFunction"), __T("CallBack=memory://") + Ztring::ToZtring((size_t)&Event_CallBackFunction) + __T(";UserHandler=memory://") + Ztring::ToZtring((size_t)this));
    MI.Option(__T("File_DvDif_Analysis"), __T("1"));
    if (!Merge_InputFileNames.empty() && (Merge_InputFileNames.front() == "-" || TransportControlsSupported())) // Only if from stdin (not supported in other cases)
        MI.Option(__T("File_Demux_Unpacketize"), __T("1"));
    if (!Merge_InputFileNames.empty() && TransportControlsSupported())
        MI.Option(__T("File_FrameIsAlwaysComplete"), __T("1"));

    if (Verbosity == 10)
        cerr << "Debug: opening (in) \"" << Ztring(FileName).To_Local() << "\"..." << endl;
    #if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
    if (Device_Command == 1 || Device_Command == 4)
    {
        size_t Count = 0;
        if (Device_Command == 4) //JSON
            cout << "[";
        #ifdef ENABLE_SIMULATOR
            for (size_t i = 0; i<SimulatorWrapper::GetDeviceCount(); i++)
            {
                auto Interface = SimulatorWrapper::GetDeviceInterface(i);
                auto DeviceName = SimulatorWrapper::GetDeviceName(i);
                auto DeviceID = i; // Use index as deviceID for simulator
                if (Device_Command == 4) //JSON
                    cout << (Count++ ? "," : "")
                         << "{\"id\":\""
                         << DeviceID
                         << "\",\"name\":\""
                         << JSON_Encode(Trim(DeviceName))
                         << "\",\"type\":\""
                         << JSON_Encode(Trim(Interface))
                         << "\"}";
                else
                {
                    cout << DeviceID << ": " << DeviceName << " [" <<  Interface << "]" << '\n';
                    Count++;
                }
            }
        #endif
        #ifdef ENABLE_AVFCTL
            for (size_t i = 0; i<AVFCtlWrapper::GetDeviceCount(); i++)
            {
                auto Interface = AVFCtlWrapper::Interface;
                auto DeviceName = AVFCtlWrapper::GetDeviceName(i);
                auto DeviceID = AVFCtlWrapper::GetDeviceID(i);
                if (DeviceID.empty())
                    continue;
                if (Device_Command == 4) //JSON
                    cout << (Count++ ? "," : "")
                         << "{\"id\":\""
                         << JSON_Encode(Trim(DeviceID))
                         << "\",\"name\":\""
                         << JSON_Encode(Trim(DeviceName))
                         << "\",\"type\":\""
                         << JSON_Encode(Trim(Interface))
                         << "\"}";
                else
                {
                    cout << DeviceID << ": " << DeviceName << " [" <<  Interface << "]" << '\n';
                    Count++;
                }
            }
        #endif

        #ifdef ENABLE_DECKLINK
            for (size_t i = 0; i<DecklinkWrapper::GetDeviceCount(); i++)
            {
                auto Interface = DecklinkWrapper::Interface;
                auto DeviceName = DecklinkWrapper::GetDeviceName(i);
                auto DeviceID = DecklinkWrapper::GetDeviceID(i);
                if (DeviceID.empty())
                    continue;
                if (Device_Command == 4) //JSON
                    cout << (Count++ ? "," : "")
                         << "{\"id\":\""
                         << JSON_Encode(Trim(DeviceID))
                         << "\",\"name\":\""
                         << JSON_Encode(Trim(DeviceName)) << "\",\"type\":\"" << JSON_Encode(Trim(Interface)) << "\"}";
                else
                {
                    cout << DeviceID << ": " << DeviceName << " [" <<  Interface << "]" << '\n';
                    Count++;
                }
            }
        #endif
        #ifdef ENABLE_LNX1394
            for (size_t i = 0; i<LinuxWrapper::GetDeviceCount(); i++)
            {
                auto Interface = LinuxWrapper::Interface;
                auto DeviceName = LinuxWrapper::GetDeviceName(i);
                auto DeviceID = LinuxWrapper::GetDeviceID(i);
                if (DeviceID.empty())
                    continue;
                if (Device_Command == 4) //JSON
                    cout << (Count++ ? "," : "")
                         << "{\"id\":\""
                         << JSON_Encode(Trim(DeviceID))
                         << "\",\"name\":\""
                         << JSON_Encode(Trim(DeviceName))
                         << "\",\"type\":\""
                         << JSON_Encode(Trim(Interface))
                         << "\"}";
                else
                {
                    cout << DeviceID << ": " << DeviceName << " [" <<  Interface << "]" << '\n';
                    Count++;
                }
            }
        #endif
        if (Device_Command == 4) //JSON
            cout << "]" << '\n';

        if (Device_Command == 1 && !Count)
            cerr << "No devices found." << '\n';

        return ReturnValue_OK;
    }
    #ifdef ENABLE_SONY9PIN
    else if (Device_Command == 5 || Device_Command == 6)
    {
        size_t Count = 0;
        if (Device_Command == 6) //JSON
            cout << "[";
        for (size_t i = 0; i<Sony9PinWrapper::GetDeviceCount(); i++)
        {
            auto ID = Sony9PinWrapper::GetDeviceID(i);
            auto Name = Sony9PinWrapper::GetDeviceName(i);
            if (Device_Command == 6) //JSON
                cout << (Count++ ? "," : "")
                     << "{\"id\":\""
                     << JSON_Encode(Trim(ID))
                     << "\",\"name\":\""
                     << JSON_Encode(Trim(Name)) << "\"}";
            else
            {
                cout << ID << (Name.empty() ? "" : (": " + Name)) << '\n';
                Count++;
            }
        }
        if (Device_Command == 6) //JSON
            cout << "]" << '\n';

        if (Device_Command == 5 && !Count)
            cerr << "No devices found." << '\n';

        return ReturnValue_OK;
    }
    #endif
    Capture = nullptr;


    #ifdef ENABLE_SONY9PIN
    if (Control_Port && strcmp(Control_Port, "native") != 0)
    {
        uint64_t Port_Pos;
        istringstream iss(Control_Port);

        iss >> Port_Pos;
        if (iss.fail() || !iss.eof())
            Port_Pos = (uint64_t)-1;

        if (Port_Pos < Sony9PinWrapper::GetDeviceCount())
            try { Controller = new Sony9PinWrapper(Port_Pos); } catch(...) {}
        else if (Sony9PinWrapper::GetDeviceIndex(Control_Port) != (size_t)-1)
            try { Controller = new Sony9PinWrapper(Control_Port); } catch(...) {}

        if (!Controller)
        {
            cerr << "Error: unable to open control port: " << Control_Port << endl;
            return ReturnValue_ERROR;
        }
    }
    #endif
    #ifdef ENABLE_DECKLINK
    if (Control_Port && strcmp(Control_Port, "native") == 0)
        DeckLinkNativeControl = true;
    #endif

    if (Device.empty() && FileName.rfind(__T("device://"), 0) == 0)
        Device = Ztring(FileName.substr(9)).To_Local();
    if (Device.empty() && Device_Command)
        Device = "0";
    if (Device.empty() && FileName.empty() && (Device_ForceCapture || !Merge_OutputFileNames.empty()))
        Device = "0";
    if (!Device.empty())
    {
        uint64_t Device_Pos, Device_Offset = 0;
        istringstream iss(Device);

        iss >> Device_Pos;
        if (iss.fail() || !iss.eof())
            Device_Pos = (uint64_t)-1;

        try {
            if (false)
                ;
            #ifdef ENABLE_SIMULATOR
            else if ((Device_Pos-=Device_Offset) < (Device_Offset=SimulatorWrapper::GetDeviceCount()))
            {
                Capture = new SimulatorWrapper(Device_Pos);
                if (((SimulatorWrapper*)Capture)->IsMatroska())
                    CaptureMode = Capture_Mode_DeckLink;
            }
            #endif
            #ifdef ENABLE_AVFCTL
            else if ((Device_Pos-=Device_Offset) < (Device_Offset=AVFCtlWrapper::GetDeviceCount()))
                Capture = new AVFCtlWrapper(Device_Pos, Controller);
            else if (AVFCtlWrapper::GetDeviceIndex(Device) != (size_t)-1)
                Capture = new AVFCtlWrapper(Device, Controller);
            #endif
            #ifdef ENABLE_DECKLINK
            else if ((Device_Pos-=Device_Offset) < (Device_Offset=DecklinkWrapper::GetDeviceCount()))
            {
                CaptureMode = Capture_Mode_DeckLink;
                Capture = new DecklinkWrapper(Device_Pos,
                                              (decklink_video_mode)DeckLinkVideoMode,
                                              (decklink_video_source)DeckLinkVideoSource,
                                              (decklink_audio_source)DeckLinkAudioSource,
                                              (decklink_timecode_format)DeckLinkTimecodeFormat,
                                              Controller,
                                              DeckLinkNativeControl);
            }
            else if (DecklinkWrapper::GetDeviceIndex(Device) != (size_t)-1)
            {
                CaptureMode = Capture_Mode_DeckLink;
                Capture = new DecklinkWrapper(Device,
                                              (decklink_video_mode)DeckLinkVideoMode,
                                              (decklink_video_source)DeckLinkVideoSource,
                                              (decklink_audio_source)DeckLinkAudioSource,
                                              (decklink_timecode_format)DeckLinkTimecodeFormat,
                                              Controller,
                                              DeckLinkNativeControl);
            }
            #endif
            #ifdef ENABLE_LNX1394
            else if ((Device_Pos-=Device_Offset) < (Device_Offset=LinuxWrapper::GetDeviceCount()))
               Capture = new LinuxWrapper(Device_Pos);
            else if (LinuxWrapper::GetDeviceIndex(Device) != (size_t)-1)
                Capture = new LinuxWrapper(Device);
            #endif
            else
            {
                cerr << "Error: device not found: " << Device << endl;
                return ReturnValue_ERROR;
            }
        }
        catch(std::exception& e)
        {
            cerr << "Error: " << e.what() << endl;
            return ReturnValue_ERROR;
        }
    }
    if (Capture)
    {
        if (Device_Command && !Device_ForceCapture)
        {
            if (Device_Command == 2)
            {
                FileWrapper Wrapper(nullptr);
                Capture->CreateCaptureSession(&Wrapper);
                this_thread::sleep_for(chrono::milliseconds(500)); // give time for the driver to retrieves status from the device
                auto Status = Capture->GetStatus();
                Capture->StopCaptureSession();
                cout << Status << '\n';
            }
            if (Device_Command == 3)
            {
                Capture->SetPlaybackMode((playback_mode)Device_Mode, Device_Speed);
                if (Foreground && Device_Speed != 0.0f)
                {
                    FileWrapper Wrapper(nullptr);
                    Capture->CreateCaptureSession(&Wrapper);
                    Capture->StartCaptureSession();
                    Capture->WaitForSessionEnd(Timeout);
                    Capture->StopCaptureSession();
                }
            }
            if (Device_Command >= 0x20)
            {
                InputControl_Char(this, Device_Command);
                if (Foreground && (Device_Command == 'R' || Device_Command == 'r' || Device_Command == 'F' || Device_Command == 'f'))
                {
                    FileWrapper Wrapper(nullptr);
                    Capture->CreateCaptureSession(&Wrapper);
                    Capture->StartCaptureSession();
                    Capture->WaitForSessionEnd(Timeout);
                    Capture->StopCaptureSession();
                }
            }
            return ReturnValue_OK;
        }
        Speed_Before = Capture->GetSpeed();
        auto InputHelper = InControl ? new thread(InputControl, this) : nullptr;
        #if defined(ENABLE_SIMULATOR) || defined(ENABLE_DECKLINK)
        if (CaptureMode == Capture_Mode_DeckLink)
        {
            uint32_t Width = 720;
            uint32_t Height = DeckLinkVideoMode == Decklink_Video_Mode_NTSC ? 486 : 576;
            uint32_t Num = DeckLinkVideoMode == Decklink_Video_Mode_NTSC ? 30000 : 25;
            uint32_t Den = DeckLinkVideoMode == Decklink_Video_Mode_NTSC ? 1001 : 1;
            uint32_t SampleRate = 48000;
            uint8_t Channels = 2;

            Wrapper = new FileWrapper(Width, Height, Num, Den, SampleRate, Channels, DeckLinkTimecodeFormat<Decklink_Timecode_Format_Max);
        }
        else
        #endif
            Wrapper = new FileWrapper(this);
        MI.Open_Buffer_Init();
        Capture->CreateCaptureSession(Wrapper);
        for (;;)
        {
            Capture->StartCaptureSession();
            if (!Device_Command)
                Capture->SetPlaybackMode(Playback_Mode_Playing, 1.0);
            else if (Device_Command != -1)
            {
                if (Device_Command == 3)
                    Capture->SetPlaybackMode((playback_mode)Device_Mode, Device_Speed);
                else if (Device_Command >= 0x20)
                    InputControl_Char(this, Device_Command);
                Device_Command = -1;
            }
            TimeOutReached = Capture->WaitForSessionEnd(Timeout);
            Capture->StopCaptureSession();
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

    return ReturnValue_OK;
}

//---------------------------------------------------------------------------
void file::Parse_Buffer(const uint8_t* Buffer, size_t Buffer_Size)
{
    MI.Open_Buffer_Continue(Buffer, Buffer_Size);
}

//---------------------------------------------------------------------------
void file::Terminate()
{
#if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
    if (Capture)
    {
        PauseRequested=false;
        Capture->SetPlaybackMode(Playback_Mode_NotPlaying,  0.0);
    }
    else
#endif
    {
        MI.Option(__T("File_RequestTerminate"), __T("1"));
    }

    TerminateRequested = true;
}

//---------------------------------------------------------------------------
file::~file()
{
    #if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
    if (Capture)
        delete Capture;

    if (Wrapper)
        delete Wrapper;
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
#if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
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
#if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
void file::RewindToTimeCode(TimeCode TC)
{
    RewindMode=Rewind_Mode_TimeCode;
    RewindTo_TC=TC;
    Capture->SetPlaybackMode(Playback_Mode_Playing, -1.0);
}
#endif

//---------------------------------------------------------------------------
#if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
void file::RewindToAbst(int Abst)
{
    RewindMode=Rewind_Mode_Abst;
    RewindTo_Abst=Abst;
    Capture->SetPlaybackMode(Playback_Mode_Playing, -1.0);
}
#endif

//---------------------------------------------------------------------------
void file::AddChange(const MediaInfo_Event_DvDif_Change_0* FrameData)
{
    #if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
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
    #if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
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
            Capture->SetPlaybackMode(Playback_Mode_Playing, 1.0);
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
                    Capture->SetPlaybackMode(Playback_Mode_Playing, 1.0);
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
                Capture->SetPlaybackMode(Playback_Mode_Playing, 1.0);
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

    coherency_flags Coherency(FrameData);
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

    if (!Merge_Out.empty())
    {
        auto Speed = abs(Speed_Before) > abs(Speed_After) ? Speed_Before : Speed_After;
        Speed_Before = Speed_After;
        Merge.AddFrameAnalysis(Merge_FilePos, FrameData, Speed);
        #if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
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
        if (Capture)
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
    #if defined(ENABLE_CAPTURE) || defined(ENABLE_SIMULATOR)
    if (DelayedPlay)
        return;
    if (RewindMode!=Rewind_Mode_None)
        return;
    #endif

    // DV frame
    if (!FrameData->StreamIDs_Size || FrameData->StreamIDs[FrameData->StreamIDs_Size-1]==-1)
    {
        if (!Merge_Out.empty() && (Merge_InputFileNames.empty() || Merge_InputFileNames[0] == "-" || Merge_InputFileNames[0].find("device://") == 0)) // Only for stdin
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
        auto& FieldData = PerSeq.FieldData[i];
        auto DUR = FrameData->DUR / 1000000000.0;
        if (FieldData.empty() || FieldData.back().StartFrameNumber + FieldData.back().Captions.size() != FrameNumber || (!FieldData.empty() && (DUR < FieldData.back().DUR - 1 || DUR > FieldData.back().DUR + 1)))
            FieldData.emplace_back(FrameData->PTS / 1000000000.0, DUR, FrameNumber);
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
