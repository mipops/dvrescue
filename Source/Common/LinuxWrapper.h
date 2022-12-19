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

#include "Common/ProcessFileWrapper.h"

#include <libraw1394/raw1394.h>
#include <libiec61883/iec61883.h>

//***************************************************************************
// Class SimulatorWrapper
//***************************************************************************

class LinuxWrapper : public BaseWrapper {
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
        int Port;
        nodeid_t Node;
        octlet_t UUID;
        std::string Vendor;
        std::string Model;

        device(int Port, nodeid_t Node, octlet_t UUID, std::string Vendor, std::string Model)
        : Port(Port), Node(Node), UUID(UUID), Vendor(Vendor), Model(Model) {};
    };

    // Constructor/Destructor
    LinuxWrapper(std::size_t DeviceIndex);
    LinuxWrapper(std::string DeviceID);
    ~LinuxWrapper();

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
    // global
    int Port = -1;
    nodeid_t Node = (nodeid_t)-1;
    octlet_t UUID = (octlet_t)-1;

    // avc1394
    raw1394handle_t CtlHandle = nullptr;

    // iec61883
    int InPlug = -1;
    int OutPlug = -1;
    int Channel = -1;
    int Bandwidth = -1;
    iec61883_dv_fb_t Frame = nullptr;
    raw1394handle_t CaptureHandle = nullptr;
    std::thread* Raw1394PoolingThread = nullptr;
    std::atomic_bool Raw1394PoolingThread_Terminate { false };
};
