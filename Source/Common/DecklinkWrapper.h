/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once

#include "ZenLib/Ztring.h"
#include <exception>
#include <atomic>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

#include <DeckLinkAPI.h>

#include "Common/ProcessFileWrapper.h"
#include "Common/ProcessFile.h"
#include "Common/Output_Mkv.h"

//***************************************************************************
// Class DecklinkWrapper
//***************************************************************************

class DecklinkWrapper : public BaseWrapper {
 public:
    // Types
    class error : public std::exception
    {
    public:
        error(std::string message) : std::exception(), message(message) {};
        virtual const char* what() const throw() { return message.c_str(); }
    private:
        std::string message;
    };

    struct device
    {
        std::string Name;
        std::string UUID;

        device(std::string Name, std::string UUID)
        : Name(Name), UUID(UUID) {};
    };

    // IDeckLinkInputCallback
    class CaptureDelegate : public IDeckLinkInputCallback {
    public:
        // Constructor/Destructor
        CaptureDelegate(FileWrapper* Wrapper, const uint32_t TimecodeFormat);

        // Functions
        ULONG AddRef();
        ULONG Release();
        HRESULT QueryInterface(REFIID, LPVOID*);
        HRESULT VideoInputFrameArrived(IDeckLinkVideoInputFrame* VideoFrame, IDeckLinkAudioInputPacket* AudioPacket);
        HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);

    private:
        FileWrapper* Wrapper;
        uint32_t TimecodeFormat;
    };

    class StatusDelegate : public IDeckLinkDeckControlStatusCallback
    {
    public:
        // Constructor/Destructor
        StatusDelegate();

        // Functions
        ULONG AddRef();
        ULONG Release();
        HRESULT QueryInterface(REFIID, LPVOID*);
        virtual HRESULT TimecodeUpdate(BMDTimecodeBCD);
        virtual HRESULT VTRControlStateChanged (BMDDeckControlVTRControlState, BMDDeckControlError);
        virtual HRESULT DeckControlEventReceived (BMDDeckControlEvent, BMDDeckControlError);
        virtual HRESULT DeckControlStatusChanged (BMDDeckControlStatusFlags flags, uint32_t mask);
    };

    // Constructor/Destructor
    DecklinkWrapper(std::size_t DeviceIndex,
                    decklink_video_mode Mode = Decklink_Video_Mode_NTSC,
                    decklink_video_source VideoSrc = Decklink_Video_Source_Composite,
                    decklink_audio_source AudioSrc = Decklink_Audio_Source_Analog,
                    decklink_timecode_format TimecodeFormat = Decklink_Timecode_Format_VITC,
                    ControllerBaseWrapper* Controller = nullptr,
                    bool Native = false);
    DecklinkWrapper(std::string DeviceID,
                    decklink_video_mode Mode = Decklink_Video_Mode_NTSC,
                    decklink_video_source VideoSrc = Decklink_Video_Source_Composite,
                    decklink_audio_source AudioSrc = Decklink_Audio_Source_Analog,
                    decklink_timecode_format TimecodeFormat = Decklink_Timecode_Format_VITC,
                    ControllerBaseWrapper* Controller = nullptr,
                    bool Native = false);
    ~DecklinkWrapper();

    // Functions
    static void Init();
    static std::size_t GetDeviceCount();
    static std::string GetDeviceName(std::size_t DeviceIndex);
    static std::string GetDeviceName(const std::string& DeviceID);
    static std::string GetDeviceID(std::size_t DeviceIndex);
    static std::size_t GetDeviceIndex(const std::string& DeviceID);
    std::string GetStatus();
    float GetSpeed();
    playback_mode GetMode();
    void CreateCaptureSession(FileWrapper* Wrapper);
    void StartCaptureSession();
    void StopCaptureSession();
    void SetPlaybackMode(playback_mode Mode, float Speed);
    bool WaitForSessionEnd(uint64_t Timeout);

    // Attributes
    static const std::string Interface;

private:
    playback_mode PlaybackMode = Playback_Mode_NotPlaying;
    bool Capture = false;

    //DeckLink
    IDeckLink* DeckLinkDevice = nullptr;
    IDeckLinkInput* DeckLinkInput = nullptr;
    IDeckLinkConfiguration* DeckLinkConfiguration = nullptr;
    CaptureDelegate* DeckLinkCaptureDelegate = nullptr;
    StatusDelegate*  DeckLinkStatusDelegate = nullptr;

    //Config
    uint32_t DeckLinkVideoMode;
    uint32_t DeckLinkVideoSource;
    uint32_t DeckLinkAudioSource;
    uint32_t DeckLinkTimecodeFormat;

    // Control
    ControllerBaseWrapper* Controller = nullptr;
    IDeckLinkDeckControl* DeckLinkDeckControl = nullptr;
};
