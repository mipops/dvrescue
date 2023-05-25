/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/DecklinkWrapper.h"
#include "Common/Merge.h"

#include <DeckLinkAPIDispatch.cpp>
#include <ctime>

using namespace std;

//---------------------------------------------------------------------------
struct frame
{
    size_t Size;
    uint8_t* Data;
};

typedef CFStringRef PlatformStr;

vector<DecklinkWrapper::device> Devices;

atomic<time_t> LastInput;
mutex ProcessFrameLock;

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
DecklinkWrapper::CaptureDelegate::CaptureDelegate(matroska_writer* Writer) : Writer(Writer)
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
        void* VideoBuffer;
        size_t VideoBufferSize = VideoFrame->GetRowBytes() * VideoFrame->GetHeight();
        if (VideoFrame->GetBytes(&VideoBuffer) != S_OK)
        {
            VideoFrame->Release();
            AudioPacket->Release();
            return E_FAIL;
        }

        void* AudioBuffer;
        size_t AudioBufferSize = AudioPacket->GetSampleFrameCount() * 2 * 32 / 8;
        if (AudioPacket->GetBytes(&AudioBuffer) != S_OK)
        {
            VideoFrame->Release();
            AudioPacket->Release();
            return E_FAIL;
        }

        Writer->write_frame((const char*)VideoBuffer, VideoBufferSize, (const char*)AudioBuffer, AudioBufferSize);
    }


    if (VideoFrame)
        VideoFrame->Release();

    if (AudioPacket)
        AudioPacket->Release();

    return S_OK;
}

//---------------------------------------------------------------------------
HRESULT DecklinkWrapper::CaptureDelegate::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags)
{
    return S_OK;
}

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
DecklinkWrapper::DecklinkWrapper(size_t DeviceIndex) : PlaybackMode(Playback_Mode_NotPlaying),
                                                       DeckLinkInput(nullptr),
                                                       DeckLinkConfiguration(nullptr),
                                                       DeckLinkCaptureDelegate(nullptr),
                                                       MatroskaWriter(nullptr)
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
}

//---------------------------------------------------------------------------
DecklinkWrapper::DecklinkWrapper(string DeviceID) : PlaybackMode(Playback_Mode_NotPlaying),
                                                    DeckLinkInput(nullptr),
                                                    DeckLinkConfiguration(nullptr),
                                                    DeckLinkCaptureDelegate(nullptr),
                                                    MatroskaWriter(nullptr)
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
}

//---------------------------------------------------------------------------
DecklinkWrapper::~DecklinkWrapper()
{
    StopCaptureSession();
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
    return PlaybackMode;
}

//---------------------------------------------------------------------------
std::string DecklinkWrapper::GetStatus()
{
    return "unknown";
}

//---------------------------------------------------------------------------
float DecklinkWrapper::GetSpeed()
{
    return 0.0f;
}

//---------------------------------------------------------------------------
void DecklinkWrapper::CreateCaptureSession(FileWrapper* Wrapper_)
{
    if (DeckLinkDevice->QueryInterface(IID_IDeckLinkInput, (void **)&DeckLinkInput) != S_OK)
        return;

    if (DeckLinkDevice->QueryInterface(IID_IDeckLinkConfiguration, (void **)&DeckLinkConfiguration) != S_OK)
    {
        DeckLinkInput->Release();
        DeckLinkInput=nullptr;

        return;
    }

    if (DeckLinkConfiguration->SetInt(bmdDeckLinkConfigVideoInputConnection, bmdVideoConnectionComposite) != S_OK ||
        DeckLinkConfiguration->SetInt(bmdDeckLinkConfigAudioInputConnection, bmdAudioConnectionAnalog) != S_OK)
    {
        DeckLinkConfiguration->Release();
        DeckLinkConfiguration=nullptr;

        DeckLinkInput->Release();
        DeckLinkInput=nullptr;
    }

    if (Merge_OutputFileName)
    {
        Output = ofstream(Merge_OutputFileName, ios_base::binary | ios_base::trunc);
        MatroskaWriter = new matroska_writer(&Output, 720, 486, 30000, 1001, false);
    }

    DeckLinkCaptureDelegate = new CaptureDelegate(MatroskaWriter);

    if (DeckLinkInput->EnableVideoInput(bmdModeNTSC, bmdFormat10BitYUV, bmdVideoInputFlagDefault) != S_OK ||
        DeckLinkInput->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType32bitInteger, 2) != S_OK ||
        DeckLinkInput->SetCallback(DeckLinkCaptureDelegate) != S_OK)
    {
        DeckLinkConfiguration->Release();
        DeckLinkConfiguration=nullptr;

        DeckLinkInput->Release();
        DeckLinkInput=nullptr;

        delete DeckLinkCaptureDelegate;
        DeckLinkCaptureDelegate=nullptr;

        if (MatroskaWriter)
        {
            delete MatroskaWriter;
            MatroskaWriter=nullptr;
        }

        return;
    }
}

//---------------------------------------------------------------------------
void DecklinkWrapper::StartCaptureSession()
{
    if (!DeckLinkInput || DeckLinkInput->StartStreams() != S_OK)
    {
        StopCaptureSession();
    }
    SetPlaybackMode(Playback_Mode_Playing, 0.0f);
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

    if (MatroskaWriter)
    {
        MatroskaWriter->close(&Output);
        Output.close();

        delete MatroskaWriter;
        MatroskaWriter=nullptr;
    }
    SetPlaybackMode(Playback_Mode_NotPlaying, 0.0f);
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
    while (PlaybackMode==Playback_Mode_Playing); //TODO: get state from device control interface

    return false;
}

//---------------------------------------------------------------------------
void DecklinkWrapper::SetPlaybackMode(playback_mode Mode, float Speed)
{
    PlaybackMode=Mode;
}
