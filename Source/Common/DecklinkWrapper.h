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
#include <string>
#include <thread>
#include <mutex>

#include <DeckLinkAPI.h>

#include "Common/ProcessFileWrapper.h"
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
        CaptureDelegate(matroska_writer* Writer);

        // Functions
        ULONG AddRef();
        ULONG Release();
        HRESULT QueryInterface(REFIID, LPVOID*);
        HRESULT VideoInputFrameArrived(IDeckLinkVideoInputFrame* VideoFrame, IDeckLinkAudioInputPacket* AudioPacket);
        HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);

    private:
        matroska_writer* Writer;
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
    DecklinkWrapper(std::size_t DeviceIndex, ControllerBaseWrapper* Controller = nullptr, bool Native = false);
    DecklinkWrapper(std::string DeviceID, ControllerBaseWrapper* Controller = nullptr, bool Native = false);
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

private:
    playback_mode PlaybackMode = Playback_Mode_NotPlaying;
    bool Capture = false;

    //DeckLink
    IDeckLink* DeckLinkDevice = nullptr;
    IDeckLinkInput* DeckLinkInput = nullptr;
    IDeckLinkConfiguration* DeckLinkConfiguration = nullptr;
    CaptureDelegate* DeckLinkCaptureDelegate = nullptr;
    StatusDelegate*  DeckLinkStatusDelegate = nullptr;

    // Control
    ControllerBaseWrapper* Controller = nullptr;
    IDeckLinkDeckControl* DeckLinkDeckControl = nullptr;

    //Output
    matroska_writer* MatroskaWriter = nullptr;
    std::ofstream Output;
};
