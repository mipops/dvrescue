/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once

#include <cstddef> //for std::size_t, native size_t isn't avaiable in obj-c++ mode
#include <string>

#include "Common/ProcessFileWrapper.h"

//***************************************************************************
// Class AVFCtlWrapper
//***************************************************************************

class AVFCtlWrapper : public BaseWrapper {
public:
    // Constructor/Destructor
    AVFCtlWrapper(std::size_t DeviceIndex);
    ~AVFCtlWrapper();

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
    void WaitForSessionEnd();

private:
    void* Ctl;
};
