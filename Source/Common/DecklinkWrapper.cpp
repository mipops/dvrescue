/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/DecklinkWrapper.h"
#include "Common/Merge.h"

#include <DeckLinkAPIDispatch.cpp>
#include <iostream>
#include <ctime>

using namespace std;

//---------------------------------------------------------------------------
static uint32_t decklink_video_modes[Decklink_Video_Mode_Max] =
{
    bmdModeNTSC,
    bmdModePAL
};

//---------------------------------------------------------------------------
static uint32_t decklink_video_sources[Decklink_Video_Source_Max] =
{
    bmdVideoConnectionSDI,
    bmdVideoConnectionHDMI,
    bmdVideoConnectionOpticalSDI,
    bmdVideoConnectionComponent,
    bmdVideoConnectionComposite,
    bmdVideoConnectionSVideo
};

//---------------------------------------------------------------------------
static uint32_t decklink_audio_sources[Decklink_Audio_Source_Max] =
{
    bmdAudioConnectionEmbedded,
    bmdAudioConnectionAESEBU,
    bmdAudioConnectionAnalog,
    bmdAudioConnectionAnalogXLR,
    bmdAudioConnectionAnalogRCA,
    bmdAudioConnectionMicrophone
};

//---------------------------------------------------------------------------
static uint32_t decklink_pixel_formats[Decklink_Pixel_Format_Max] =
{
    bmdFormatUnspecified,
    bmdFormat8BitYUV,
    bmdFormat10BitYUV,
    bmdFormat8BitARGB,
    bmdFormat8BitBGRA,
    bmdFormat10BitRGB
};

//---------------------------------------------------------------------------
static uint32_t decklink_timecode_formats[Decklink_Timecode_Format_Max] =
{
    bmdTimecodeRP188VITC1,
    bmdTimecodeRP188VITC2,
    bmdTimecodeRP188LTC,
    bmdTimecodeRP188HighFrameRate,
    bmdTimecodeRP188Any,
    bmdTimecodeVITC,
    bmdTimecodeVITCField2,
    bmdTimecodeSerial
};

typedef CFStringRef PlatformStr;

vector<DecklinkWrapper::device> Devices;

atomic<time_t> LastInput;
mutex ProcessFrameLock;

atomic<bool> DeckLinkDeckControlConnected;

//---------------------------------------------------------------------------
string PlatformStr2StdStr(PlatformStr Str)
{
    if (!Str)
        return string();

    string StdStr(CFStringGetCStringPtr(Str, kCFStringEncodingUTF8));
    CFRelease(Str);

    return StdStr;
}

//---------------------------------------------------------------------------
DecklinkWrapper::CaptureDelegate::CaptureDelegate(FileWrapper* Wrapper, const uint32_t TimecodeFormat) : Wrapper(Wrapper), TimecodeFormat(TimecodeFormat)
{
}

//---------------------------------------------------------------------------
ULONG DecklinkWrapper::CaptureDelegate::AddRef()
{
    return 0x1;
}

//---------------------------------------------------------------------------
ULONG DecklinkWrapper::CaptureDelegate::Release()
{
    return 0x1;
}

//---------------------------------------------------------------------------
HRESULT DecklinkWrapper::CaptureDelegate::QueryInterface(REFIID, LPVOID*)
{
    return E_NOINTERFACE;
}

//---------------------------------------------------------------------------
HRESULT DecklinkWrapper::CaptureDelegate::VideoInputFrameArrived(IDeckLinkVideoInputFrame* VideoFrame, IDeckLinkAudioInputPacket* AudioPacket)
{
    const lock_guard<mutex> Lock(ProcessFrameLock);
    if (VideoFrame && AudioPacket)
    {
        LastInput = time(NULL);

        void* VideoBuffer;
        size_t VideoBufferSize = VideoFrame->GetRowBytes() * VideoFrame->GetHeight();
        if (VideoFrame->GetBytes(&VideoBuffer) != S_OK)
            return E_FAIL;

        void* AudioBuffer;
        size_t AudioBufferSize = AudioPacket->GetSampleFrameCount() * 2 * 32 / 8;
        if (AudioPacket->GetBytes(&AudioBuffer) != S_OK)
            return E_FAIL;

        TimeCode TC;
        if (TimecodeFormat != (uint32_t)-1)
        {
            IDeckLinkTimecode*	DeckLinkTimecode;
            if (VideoFrame->GetTimecode(TimecodeFormat, &DeckLinkTimecode) == S_OK)
            {
                DeckLinkTimecode->GetComponents(&TC.Hours, &TC.Minutes, &TC.Seconds, &TC.Frames);
                TC.DropFrame = (DeckLinkTimecode->GetFlags() & bmdTimecodeIsDropFrame) != 0;
            }
        }

        uint8_t PixelFormat = (uint8_t)Decklink_Pixel_Format_Unspecified;
        switch (VideoFrame->GetPixelFormat())
        {
            case bmdFormat8BitYUV:
                PixelFormat = (uint8_t)Decklink_Pixel_Format_8BitYUV; break;
            case bmdFormat10BitYUV:
                PixelFormat = (uint8_t)Decklink_Pixel_Format_10BitYUV; break;
            case bmdFormat8BitARGB:
                PixelFormat = (uint8_t)Decklink_Pixel_Format_8BitARGB; break;
            case bmdFormat8BitBGRA:
                PixelFormat = (uint8_t)Decklink_Pixel_Format_8BitBGRA; break;
            case bmdFormat10BitRGB:
                PixelFormat = (uint8_t)Decklink_Pixel_Format_10BitRGB; break;
            default:;
        }

        decklink_frame Buffer = {
            .Width = (uint32_t)VideoFrame->GetWidth(),
            .Height = (uint32_t)VideoFrame->GetHeight(),
            .Pixel_Format = PixelFormat,
            .Video_Buffer = (uint8_t*)VideoBuffer,
            .Video_Buffer_Size = VideoBufferSize,
            .Audio_Buffer = (uint8_t*)AudioBuffer,
            .Audio_Buffer_Size = AudioBufferSize,
            .TC = TC,
        };

        Wrapper->Parse_Buffer((const uint8_t*)&Buffer, sizeof(Buffer));
    }

    return S_OK;
}

//---------------------------------------------------------------------------
HRESULT DecklinkWrapper::CaptureDelegate::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags)
{
    return S_OK;
}

//---------------------------------------------------------------------------
DecklinkWrapper::StatusDelegate::StatusDelegate()
{
}

//---------------------------------------------------------------------------
ULONG DecklinkWrapper::StatusDelegate::AddRef()
{
    return 0x1;
}

//---------------------------------------------------------------------------
ULONG DecklinkWrapper::StatusDelegate::Release()
{
    return 0x1;
}

//---------------------------------------------------------------------------
HRESULT DecklinkWrapper::StatusDelegate::QueryInterface(REFIID, LPVOID*)
{
    return E_NOINTERFACE;
}

//---------------------------------------------------------------------------
HRESULT DecklinkWrapper::StatusDelegate::TimecodeUpdate(BMDTimecodeBCD)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------
HRESULT DecklinkWrapper::StatusDelegate::VTRControlStateChanged(BMDDeckControlVTRControlState, BMDDeckControlError)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------
HRESULT DecklinkWrapper::StatusDelegate::DeckControlEventReceived(BMDDeckControlEvent, BMDDeckControlError)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------
HRESULT DecklinkWrapper::StatusDelegate::DeckControlStatusChanged(BMDDeckControlStatusFlags flags, uint32_t mask)
{
    if ((mask & bmdDeckControlStatusDeckConnected) && (flags & bmdDeckControlStatusDeckConnected))
        DeckLinkDeckControlConnected = true;

    return S_OK;
}

//---------------------------------------------------------------------------
const string DecklinkWrapper::Interface = "DeckLink";

//---------------------------------------------------------------------------
void DecklinkWrapper::Init()
{
    Devices.clear();

    IDeckLinkIterator* DeckLinkIterator = CreateDeckLinkIteratorInstance();
    if (!DeckLinkIterator)
        return;

    IDeckLink* DeckLink;
    while (DeckLinkIterator->Next(&DeckLink) == S_OK)
    {
        IDeckLinkProfileAttributes* Attrs;

        string Name, UUID;
        if (DeckLink->QueryInterface(IID_IDeckLinkProfileAttributes, (void **)&Attrs) == S_OK)
        {
            PlatformStr DeviceName;
            if (Attrs->GetString(BMDDeckLinkDisplayName, &DeviceName) == S_OK)
                Name = PlatformStr2StdStr(DeviceName);

            PlatformStr DeviceUUID;
            if (Attrs->GetString(BMDDeckLinkDeviceHandle, &DeviceUUID) == S_OK)
                UUID = PlatformStr2StdStr(DeviceUUID);

            Devices.push_back(device(Name, UUID));

            Attrs->Release();
        }

        DeckLink->Release();
    }

    DeckLinkIterator->Release();
}

//---------------------------------------------------------------------------
DecklinkWrapper::DecklinkWrapper(size_t DeviceIndex,
                                 decklink_video_mode Mode,
                                 decklink_video_source VideoSrc,
                                 decklink_audio_source AudioSrc,
                                 decklink_timecode_format TimecodeFormat,
                                 ControllerBaseWrapper* Controller,
                                 bool Native) : Controller(Controller)
{
    IDeckLinkIterator* DeckLinkIterator = CreateDeckLinkIteratorInstance();
    if (!DeckLinkIterator)
        throw error("Unable to initialize DeckLink API.");

    for (size_t Pos=0; DeckLinkIterator->Next(&DeckLinkDevice) == S_OK; Pos++)
    {
        if (DeviceIndex==Pos)
            break;

        DeckLinkDevice->Release();
        DeckLinkDevice=nullptr;
    }

    DeckLinkIterator->Release();

    if (!DeckLinkDevice)
        throw error("Device not found.");

    DeckLinkVideoMode = decklink_video_modes[Mode < Decklink_Video_Mode_Max ? Mode : Decklink_Video_Mode_NTSC];
    DeckLinkVideoSource = decklink_video_sources[VideoSrc < Decklink_Video_Source_Max ? VideoSrc : Decklink_Video_Source_SDI];
    DeckLinkAudioSource = decklink_audio_sources[AudioSrc < Decklink_Audio_Source_Max ? AudioSrc : Decklink_Audio_Source_Embedded];
    DeckLinkTimecodeFormat = TimecodeFormat < Decklink_Timecode_Format_Max ? decklink_timecode_formats[TimecodeFormat] : (uint32_t)-1;

    if (!Controller && Native)
    {
        BMDDeckControlError Error;
        DeckLinkDeckControlConnected = false;

        DeckLinkDevice->QueryInterface(IID_IDeckLinkDeckControl, (void **)&DeckLinkDeckControl);

        if (!DeckLinkDeckControl)
        {
            DeckLinkDevice->Release();
            throw error("Unable to initialize DeckLink DeckControl API.");
        }

        DeckLinkStatusDelegate = new StatusDelegate();
        DeckLinkDeckControl->SetCallback(DeckLinkStatusDelegate);
        if (DeckLinkDeckControl->Open(30000, 1001, true, &Error) != S_OK)
        {
            DeckLinkDevice->Release();
            DeckLinkDeckControl->Release();
            delete DeckLinkStatusDelegate;
            throw error("Could not open serial port.");
        }

        time_t Start = time(NULL);
        while (!DeckLinkDeckControlConnected)
        {
            if (difftime(time(NULL), Start) > 10)
            {
                DeckLinkDeckControl->Close(false);

                DeckLinkDevice->Release();
                DeckLinkDeckControl->Release();
                delete DeckLinkStatusDelegate;
                throw error("Could not open serial port.");
            }
            this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

//---------------------------------------------------------------------------
DecklinkWrapper::DecklinkWrapper(string DeviceID,
                                 decklink_video_mode Mode,
                                 decklink_video_source VideoSrc,
                                 decklink_audio_source AudioSrc,
                                 decklink_timecode_format TimecodeFormat,
                                 ControllerBaseWrapper* Controller,
                                 bool Native) : Controller(Controller)
{
    IDeckLinkIterator* DeckLinkIterator = CreateDeckLinkIteratorInstance();
    if (!DeckLinkIterator)
        throw error("Unable to initialize DeckLink API.");

    while (DeckLinkIterator->Next(&DeckLinkDevice) == S_OK)
    {
        IDeckLinkProfileAttributes* Attrs;

        string UUID;
        if (DeckLinkDevice->QueryInterface(IID_IDeckLinkProfileAttributes, (void **)&Attrs) == S_OK)
        {
            PlatformStr DeviceUUID;
            if (Attrs->GetString(BMDDeckLinkDeviceHandle, &DeviceUUID) == S_OK)
                UUID = PlatformStr2StdStr(DeviceUUID);

            Attrs->Release();
        }

        if (DeviceID==UUID)
            break;

        DeckLinkDevice->Release();
        DeckLinkDevice=nullptr;
    }

    DeckLinkIterator->Release();

    if (!DeckLinkDevice)
        throw error("Device not found.");

    DeckLinkVideoMode = decklink_video_modes[Mode < Decklink_Video_Mode_Max ? Mode : Decklink_Video_Mode_NTSC];
    DeckLinkVideoSource = decklink_video_sources[VideoSrc < Decklink_Video_Source_Max ? VideoSrc : Decklink_Video_Source_SDI];
    DeckLinkAudioSource = decklink_audio_sources[AudioSrc < Decklink_Audio_Source_Max ? AudioSrc : Decklink_Audio_Source_Embedded];
    DeckLinkTimecodeFormat = TimecodeFormat < Decklink_Timecode_Format_Max ? decklink_timecode_formats[TimecodeFormat] : (uint32_t)-1;

    if (!Controller && Native)
    {
        BMDDeckControlError Error;
        DeckLinkDeckControlConnected = false;

        DeckLinkDevice->QueryInterface(IID_IDeckLinkDeckControl, (void **)&DeckLinkDeckControl);

        if (!DeckLinkDeckControl)
        {
            DeckLinkDevice->Release();
            throw error("Unable to initialize DeckLink DeckControl API.");
        }

        DeckLinkStatusDelegate = new StatusDelegate();
        DeckLinkDeckControl->SetCallback(DeckLinkStatusDelegate);
        if (DeckLinkDeckControl->Open(30000, 1001, true, &Error) != S_OK)
        {
            DeckLinkDevice->Release();
            DeckLinkDeckControl->Release();
            delete DeckLinkStatusDelegate;
            throw error("Could not open serial port.");
        }

        time_t Start = time(NULL);
        while (!DeckLinkDeckControlConnected)
        {
            if (difftime(time(NULL), Start) > 10)
            {
                DeckLinkDeckControl->Close(false);

                DeckLinkDevice->Release();
                DeckLinkDeckControl->Release();
                delete DeckLinkStatusDelegate;
                throw error("Could not open serial port.");
            }
            this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

//---------------------------------------------------------------------------
DecklinkWrapper::~DecklinkWrapper()
{
    if (Capture)
        StopCaptureSession();

    if (DeckLinkDevice)
        DeckLinkDevice->Release();

    if (DeckLinkDeckControl)
    {
        DeckLinkDeckControl->Close(false);
        DeckLinkDeckControl->Release();
    }

    if (DeckLinkStatusDelegate)
        delete DeckLinkStatusDelegate;
}

//---------------------------------------------------------------------------
size_t DecklinkWrapper::GetDeviceCount()
{
    Init();

    return Devices.size();
}

//---------------------------------------------------------------------------
string DecklinkWrapper::GetDeviceName(size_t DeviceIndex)
{
    Init();

    if (DeviceIndex >= Devices.size())
        return "";

    return Devices[DeviceIndex].Name;
}

//---------------------------------------------------------------------------
string DecklinkWrapper::GetDeviceName(const string& DeviceID)
{
    Init();

    for (size_t Pos = 0; Pos < Devices.size(); Pos++)
    {
        if (Devices[Pos].UUID == DeviceID)
            return Devices[Pos].Name;
    }

    return "";
}

//---------------------------------------------------------------------------
string DecklinkWrapper::GetDeviceID(size_t DeviceIndex)
{
    Init();

    if (DeviceIndex >= Devices.size())
        return "";

    return Devices[DeviceIndex].UUID;
}

//---------------------------------------------------------------------------
size_t DecklinkWrapper::GetDeviceIndex(const string& DeviceID)
{
    Init();

    for (size_t Pos = 0; Pos < Devices.size(); Pos++)
    {
        if (Devices[Pos].UUID == DeviceID)
            return Pos;
    }

    return (size_t)-1;
}

//---------------------------------------------------------------------------
playback_mode DecklinkWrapper::GetMode()
{
    if (Controller)
        return Controller->GetMode();
    else if (DeckLinkDeckControl)
    {
        BMDDeckControlMode Mode = 0;
        BMDDeckControlVTRControlState State = 0;
        BMDDeckControlStatusFlags Flags = 0;
        DeckLinkDeckControl->GetCurrentState(&Mode, &State, &Flags);
        if (State == bmdDeckControlVTRControlPlaying ||
            State == bmdDeckControlVTRControlShuttleForward ||
            State == bmdDeckControlVTRControlShuttleReverse ||
            State == bmdDeckControlVTRControlJogForward ||
            State == bmdDeckControlVTRControlJogReverse)
                return Playback_Mode_Playing;
            else
                return Playback_Mode_NotPlaying;
    }

    return PlaybackMode;
}

//---------------------------------------------------------------------------
std::string DecklinkWrapper::GetStatus()
{
    if (Controller)
        return Controller->GetStatus();
    else if (DeckLinkDeckControl)
    {
        BMDDeckControlMode Mode = 0;
        BMDDeckControlVTRControlState State = 0;
        BMDDeckControlStatusFlags Flags = 0;
        DeckLinkDeckControl->GetCurrentState(&Mode, &State, &Flags);
        if (State == bmdDeckControlVTRControlPlaying)
            return "playing";
        else if (State == bmdDeckControlVTRControlShuttleForward)
            return "playing";
        else if (State == bmdDeckControlVTRControlShuttleReverse)
            return "playing (reverse)";
        else if (State == bmdDeckControlVTRControlJogForward)
            return "playing";
        else if (State == bmdDeckControlVTRControlJogReverse)
            return "playing (reverse)";
    }
    return "unknown";
}

//---------------------------------------------------------------------------
float DecklinkWrapper::GetSpeed()
{
    if (Controller)
        return Controller->GetSpeed();
    else if (DeckLinkDeckControl)
    {
        BMDDeckControlMode Mode = 0;
        BMDDeckControlVTRControlState State = 0;
        BMDDeckControlStatusFlags Flags = 0;
        DeckLinkDeckControl->GetCurrentState(&Mode, &State, &Flags);
        if (State == bmdDeckControlVTRControlPlaying)
            return 1.0f;
        else if (State == bmdDeckControlVTRControlShuttleForward)
            return 2.0f;
        else if (State == bmdDeckControlVTRControlShuttleReverse)
            return -2.0f;
        else if (State == bmdDeckControlVTRControlJogForward)
            return 1.5f;
        else if (State == bmdDeckControlVTRControlJogReverse)
            return -1.5f;
    }

    return 0.0f;
}

//---------------------------------------------------------------------------
void DecklinkWrapper::CreateCaptureSession(FileWrapper* Wrapper_)
{
    if (DeckLinkDevice->QueryInterface(IID_IDeckLinkInput, (void **)&DeckLinkInput) != S_OK)
        return;

    bool VideoModeIsSupported = false;
    if (DeckLinkInput->DoesSupportVideoMode(DeckLinkVideoSource,
                                            DeckLinkVideoMode,
                                            bmdFormat10BitYUV,
                                            bmdNoVideoInputConversion,
                                            bmdVideoInputFlagDefault,
                                            NULL,
                                            &VideoModeIsSupported) != S_OK || !VideoModeIsSupported)
    {
        DeckLinkInput->Release();
        DeckLinkInput=nullptr;
        return;
    }

    if (DeckLinkDevice->QueryInterface(IID_IDeckLinkConfiguration, (void **)&DeckLinkConfiguration) != S_OK)
    {
        DeckLinkInput->Release();
        DeckLinkInput=nullptr;

        return;
    }

    if (DeckLinkConfiguration->SetInt(bmdDeckLinkConfigVideoInputConnection, DeckLinkVideoSource) != S_OK ||
        DeckLinkConfiguration->SetInt(bmdDeckLinkConfigAudioInputConnection, DeckLinkAudioSource) != S_OK)
    {
        DeckLinkConfiguration->Release();
        DeckLinkConfiguration=nullptr;

        DeckLinkInput->Release();
        DeckLinkInput=nullptr;
    }

    uint8_t Channels = 2;
    DeckLinkCaptureDelegate = new CaptureDelegate(Wrapper_, DeckLinkTimecodeFormat);

    if (DeckLinkInput->EnableVideoInput(DeckLinkVideoMode, bmdFormat10BitYUV, bmdVideoInputFlagDefault) != S_OK ||
        DeckLinkInput->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType32bitInteger, Channels) != S_OK ||
        DeckLinkInput->SetCallback(DeckLinkCaptureDelegate) != S_OK)
    {
        DeckLinkConfiguration->Release();
        DeckLinkConfiguration=nullptr;

        DeckLinkInput->Release();
        DeckLinkInput=nullptr;

        delete DeckLinkCaptureDelegate;
        DeckLinkCaptureDelegate=nullptr;

        return;
    }

    Capture = true;
}

//---------------------------------------------------------------------------
void DecklinkWrapper::StartCaptureSession()
{
    if (!DeckLinkInput || DeckLinkInput->StartStreams() != S_OK)
    {
        StopCaptureSession();
    }
    SetPlaybackMode(Playback_Mode_Playing, 1.0f);
}

//---------------------------------------------------------------------------
void DecklinkWrapper::StopCaptureSession()
{
    if (DeckLinkConfiguration)
    {
        DeckLinkConfiguration->Release();
        DeckLinkConfiguration=nullptr;
    }

    if (DeckLinkInput)
    {
        DeckLinkInput->StopStreams();
        DeckLinkInput->DisableVideoInput();
        DeckLinkInput->DisableAudioInput();
        DeckLinkInput->SetCallback(nullptr);
        DeckLinkInput->Release();
        DeckLinkInput=nullptr;
    }

    if (DeckLinkCaptureDelegate)
    {
        delete DeckLinkCaptureDelegate;
        DeckLinkCaptureDelegate=nullptr;
    }

    SetPlaybackMode(Playback_Mode_NotPlaying, 0.0f);
    Capture = false;
}

//---------------------------------------------------------------------------
bool DecklinkWrapper::WaitForSessionEnd(uint64_t Timeout)
{
    LastInput = time(NULL);
    do
    {
        if (Timeout)
        {
            if (difftime(time(NULL), LastInput) > Timeout)
                return true;
        }
        this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    while ((PlaybackMode = GetMode()) == Playback_Mode_Playing);

    return false;
}

//---------------------------------------------------------------------------
void DecklinkWrapper::SetPlaybackMode(playback_mode Mode, float Speed)
{
    if (Controller)
        Controller->SetPlaybackMode(Mode, Speed);
    else if (DeckLinkDeckControl)
    {
        BMDDeckControlError Error = bmdDeckControlNoError;
        switch (Mode)
        {
            case Playback_Mode_Playing:
                if (Speed > 1.0f)
                    DeckLinkDeckControl->Shuttle(25.0f, &Error);
                if (Speed == 1.0f)
                    DeckLinkDeckControl->Play(&Error);
                if (Speed > 0.0f)
                    DeckLinkDeckControl->Jog(25.0f, &Error);
                else if (Speed < 1.0f)
                    DeckLinkDeckControl->Shuttle(-25.0f, &Error);
                else if (Speed < 0.0f)
                    DeckLinkDeckControl->Jog(-25.0f, &Error);
                else
                    DeckLinkDeckControl->Stop(&Error);
            break;
            case Playback_Mode_NotPlaying:
                if (Speed > 1.0f)
                    DeckLinkDeckControl->FastForward(false, &Error);
                if (Speed > 0.0f)
                    DeckLinkDeckControl->Play(&Error);
                else if (Speed < 0.0f)
                    DeckLinkDeckControl->Rewind(false, &Error);
                else
                    DeckLinkDeckControl->Stop(&Error);
            break;
        }

        if (Error != bmdDeckControlNoError)
            return;
    }

    PlaybackMode=Mode;
}
