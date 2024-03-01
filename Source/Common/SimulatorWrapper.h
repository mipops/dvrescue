/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once

#include "ZenLib/Ztring.h"
#include <string>

#include "Common/ProcessFileWrapper.h"

//***************************************************************************
// Class SimulatorWrapper
//***************************************************************************

class SimulatorWrapper : public BaseWrapper {
 public:
    // Constructor/Destructor
    SimulatorWrapper(std::size_t DeviceIndex);
    ~SimulatorWrapper();

    // Functions
    static std::size_t GetDeviceCount();
    static std::string GetDeviceName(std::size_t DeviceIndex);
    std::string GetStatus();
    float GetSpeed();
    playback_mode GetMode();
    void CreateCaptureSession(FileWrapper* Wrapper);
    void StartCaptureSession();
    void StopCaptureSession();
    void SetPlaybackMode(playback_mode Mode, float Speed);
    bool WaitForSessionEnd(uint64_t Timeout);
    bool IsMatroska();

    // Attributes
    static const std::string Interface;

private:
    void* Ctl;
};
