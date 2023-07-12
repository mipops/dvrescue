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

//---------------------------------------------------------------------------
class Controller;

//***************************************************************************
// Class DecklinkWrapper
//***************************************************************************

class Sony9PinWrapper : public ControllerBaseWrapper {
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

    // Constructor/Destructor
    Sony9PinWrapper(std::size_t DeviceIndex);
    Sony9PinWrapper(std::string DeviceName);
    ~Sony9PinWrapper();

    // Functions
    static void Init();
    static std::size_t GetDeviceCount();
    static std::string GetDeviceName(std::size_t DeviceIndex);
    static std::size_t GetDeviceIndex(const std::string& DeviceName);
    std::string GetStatus();
    float GetSpeed();
    playback_mode GetMode();
    void SetPlaybackMode(playback_mode Mode, float Speed);

private:
    bool Ack();
};
