/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once

#include <cstddef> //for std::size_t, native size_t isn't avaiable in obj-c++ mode
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

#include "ThirdParty/TimeCode/TimeCode.h"
#include "Common/SignalStats.h"

class file;
#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
class matroska_writer;
#endif

//***************************************************************************
// Enums
//***************************************************************************

enum playback_mode {
    Playback_Mode_NotPlaying = 0,
    Playback_Mode_Playing
};

//***************************************************************************
// Structures
//***************************************************************************

#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
struct decklink_frame
{
    uint32_t                    Width = 0;
    uint32_t                    Height = 0;
    uint8_t                     Pixel_Format = 0;
    uint8_t*                    Video_Buffer = nullptr;
    size_t                      Video_Buffer_Size = 0;
    uint8_t*                    Audio_Buffer = nullptr;
    size_t                      Audio_Buffer_Size = 0;
    TimeCode                    TC = TimeCode();
};

struct decklink_framesinfo {
    struct frame {
        SignalStats::Stats st;
        TimeCode tc;
        bool tc_r;
        uint8_t tc_nc;
        double pts;
        double dur;
    };

    uint32_t video_width = 0;
    uint32_t video_height = 0;
    uint32_t video_rate_num = 0;
    uint32_t video_rate_den = 0;
    uint8_t pixel_format = 0;
    uint8_t audio_channels = 0;
    uint32_t audio_rate = 0;
    std::vector<frame> frames;
};

struct matroska_output
{
    matroska_writer* Writer = nullptr;
    std::ofstream* Output = nullptr;
};
#endif

//***************************************************************************
// Class FileWrapper
//***************************************************************************

class FileWrapper {
public:
    FileWrapper(file* File); // Constructor for DV/MediaInfo Interface
    #if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
    FileWrapper(int Width, int Height, int Framerate_Num, int Framerate_Den, int SampleRate, int Channels, bool Has_Timecode = false); // Constructor for Decklink/Matroska Interface
    ~FileWrapper();
    #endif
    void Parse_Buffer(const uint8_t* Buffer, std::size_t Buffer_Size);

    #if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
    decklink_framesinfo FramesInfo;
    #endif

 private:
    file* File;
    #if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
    bool IsMatroska = false;
    bool HasTimecode = false;
    std::vector<matroska_output> Outputs;
    size_t FrameCount = 0;
    ostream* Log;
    #endif
};

//***************************************************************************
// Device Capabilities
//***************************************************************************

// Transport capabilities reported by the device via AV/C SPECIFIC_INQUIRY
// or detected from the device interface
struct device_capabilities {
    // Device identification
    std::string Vendor;
    std::string Model;
    std::string Interface;          // "DV" (FireWire), "DeckLink", "AVFoundation"
    std::string UniqueID;

    // Transport capabilities
    bool CanPlay = false;           // Normal forward play
    bool CanRecord = false;         // Has record capability (AV/C RECORD opcode)
    bool CanReverse = false;        // X1 reverse play (-1.0x)
    bool CanSlowReverse = false;    // Slow reverse play (-0.5x)
    bool CanFastReverse = false;    // Fast reverse play (-2.0x)
    bool CanSlowForward = false;    // Slow forward play (0.5x)
    bool CanFastForward = false;    // Fast forward play (2.0x)
    bool CanShuttle = false;        // Variable-speed shuttle
    bool CanJog = false;            // Frame-by-frame jog
    bool CanPause = false;          // Play-pause (still frame)
    bool CanWind = false;           // Non-play fast-forward/rewind (no video output)

    // Reverse frame output capability
    //   True  = device supports PLAY reverse (AV/C PLAY opcode with reverse
    //           operand) which per the AV/C VCR spec means the device outputs
    //           DV frames over the isochronous channel while transporting in
    //           reverse. This is distinct from WIND rewind (opcode 0xC4) which
    //           only moves the tape mechanically without outputting video.
    //   False = device can only WIND (mechanical rewind); no frames during reverse.
    bool CanOutputReverse = false;

    // Supported reverse play speeds (negative values)
    std::vector<float> SupportedReverseSpeeds;
    // Supported forward play speeds
    std::vector<float> SupportedForwardSpeeds;

    // Media info
    bool HasTape = false;           // Tape loaded (MEDIUM INFO cassette != 0x60)
    bool IsProtected = false;       // Write-protected
    uint8_t CassetteType = 0;      // AV/C MEDIUM INFO cassette type byte

    // Format info from OUTPUT SIGNAL MODE (opcode 0x78)
    std::string SignalMode;         // "SD-DVCR/525-60", "SD-DVCR/625-50", etc.
    uint8_t OutputSignalMode = 0xFF;// Raw byte from OUTPUT_SIGNAL_MODE response

    bool Probed = false;            // True if capabilities were actually queried

    // Check if a specific speed is supported
    bool SupportsSpeed(float Speed) const
    {
        if (!Probed)
            return true; // Assume supported if not probed
        auto& Speeds = (Speed < 0) ? SupportedReverseSpeeds : SupportedForwardSpeeds;
        for (auto S : Speeds)
            if (S == Speed)
                return true;
        // Check generic capabilities
        if (Speed == 1.0f) return CanPlay;
        if (Speed == -1.0f) return CanReverse;
        if (Speed > 0 && Speed < 1.0f) return CanSlowForward;
        if (Speed < 0 && Speed > -1.0f) return CanSlowReverse;
        if (Speed > 1.0f) return CanFastForward;
        if (Speed < -1.0f) return CanFastReverse;
        return false;
    }

    // Check if device can output DV frames during reverse playback
    bool CanReverseWithOutput() const
    {
        if (!Probed)
            return true; // Assume yes if not probed
        return CanOutputReverse;
    }

    // Decode cassette type byte to human-readable string
    static std::string CassetteTypeName(uint8_t Type)
    {
        switch (Type)
        {
            case 0x31: return "MiniDV/Standard DV";
            case 0x33: return "DV Medium";
            case 0x41: return "MicroMV";
            case 0x22: return "VHS";
            case 0x23: return "VHS-C";
            case 0x12: return "8mm";
            case 0x60: return "No cassette";
            case 0x7E: return "Unknown";
            default:   return "Other (0x" + std::string(1, "0123456789ABCDEF"[(Type>>4)&0xF]) +
                                             std::string(1, "0123456789ABCDEF"[Type&0xF]) + ")";
        }
    }

    // Decode output signal mode byte to human-readable string
    static std::string SignalModeName(uint8_t Mode)
    {
        switch (Mode)
        {
            case 0x00: return "SD-DVCR/525-60 (NTSC)";
            case 0x01: return "SDL-DVCR/525-60";
            case 0x02: return "HD-DVCR/1125-60";
            case 0x04: return "SD-DVCR/625-50 (PAL)";
            case 0x05: return "SDL-DVCR/625-50";
            case 0x06: return "HD-DVCR/1250-50";
            case 0xFF: return "Not available";
            default:   return "Unknown (0x" + std::string(1, "0123456789ABCDEF"[(Mode>>4)&0xF]) +
                                               std::string(1, "0123456789ABCDEF"[Mode&0xF]) + ")";
        }
    }
};

//***************************************************************************
// Class BaseWrapper
//***************************************************************************

class BaseWrapper {
public:
    // Constructor/Destructor
    virtual ~BaseWrapper() = 0;

    // Functions
    virtual std::string GetStatus() = 0;
    virtual float GetSpeed() = 0;
    virtual playback_mode GetMode() = 0;
    virtual void CreateCaptureSession(FileWrapper* Wrapper) = 0;
    virtual void StartCaptureSession() = 0;
    virtual void StopCaptureSession() = 0;
    virtual void SetPlaybackMode(playback_mode Mode, float Speed) = 0;
    virtual bool WaitForSessionEnd(uint64_t Timeout) = 0;
    virtual device_capabilities GetCapabilities() { return device_capabilities(); }

    // Termination flag for responsive Ctrl-C (#783)
    bool* TerminateFlag = nullptr;
};
inline BaseWrapper::~BaseWrapper() {}

//TODO: split Controller/Capture logics
class ControllerBaseWrapper {
public:
    // Constructor/Destructor
    virtual ~ControllerBaseWrapper() = 0;

    // Functions
    virtual std::string GetStatus() = 0;
    virtual float GetSpeed() = 0;
    virtual playback_mode GetMode() = 0;
    virtual void SetPlaybackMode(playback_mode Mode, float Speed) = 0;
};
inline ControllerBaseWrapper::~ControllerBaseWrapper() {}

//***************************************************************************
// Utils
//***************************************************************************

//---------------------------------------------------------------------------
static std::string to_string(float x)
{
    long long p = 0;
    char* s = (char*)&p + sizeof(long long) - 2;
    int decimals;
    int units;
    static int Divisor = 10; // 1 decimal
    if (x < 0)
    {
        decimals = (int)(x * -Divisor + 0.5) % Divisor;
        units = (int)(-1 * x);
    }
    else {
        decimals = (int)(x * Divisor + 0.5) % Divisor;
        units = (int)x;
    }

    *--s = (decimals % 10) + '0'; // 1 decimal
    *--s = '.';

    do
    {
        *--s = (units % 10) + '0';
        units /= 10;
    } while (units > 0);

    if (x < 0)
        *--s = '-';
    return std::string(s);
}
