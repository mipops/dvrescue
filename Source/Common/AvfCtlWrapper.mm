/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

#include "Common/AvfCtlWrapper.h"

// Import the Swift-generated Objective-C header.
// When building with swiftc, this header is emitted via -emit-objc-header.
// The module name defaults to "DVRescue" but can be overridden by the build system.
#if __has_include("DVRescue-Swift.h")
    #import "DVRescue-Swift.h"
#elif __has_include("dvrescue-Swift.h")
    #import "dvrescue-Swift.h"
#else
    // Fallback: import the legacy Objective-C header directly.
    // This path is used when the Swift module header isn't available
    // (e.g., non-Swift builds or mixed build systems).
    #import "Common/AvfCtl.h"
#endif

using namespace std;

// MARK: - AVFCtlBufferReceiver (Obj-C++ bridge for C++ FileWrapper)
// This class must remain in Obj-C++ because it holds a raw C++ pointer
// (FileWrapper*) and calls C++ methods from an AVFoundation callback.

@interface AVFCtlBufferReceiver : NSObject <ReceiverTimer>
@property (retain,nonatomic) NSMutableData *output_data;
@property (assign,nonatomic) FileWrapper *output_wrapper;
@property (atomic, strong) NSDate *last_input;

- (NSDate*) lastInput;
- (void) setLastInput: (NSDate*) toDate;
- (id) initWithFileWrapper:(FileWrapper*)wrapper;
- (void) captureOutput:(AVCaptureOutput*)captureOutput
  didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection*)connection;
- (void) captureOutput:(AVCaptureOutput*)captureOutput
    didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection*)connection;
@end

@implementation AVFCtlBufferReceiver
- (void) setLastInput: (NSDate*) toDate
{
    _last_input = toDate;
}

- (NSDate*) lastInput
{
    return _last_input;
}

- (id) initWithFileWrapper:(FileWrapper*)wrapper
{
    self = [super init];

    if (self) {
        _output_wrapper = wrapper;
        _output_data = [NSMutableData dataWithLength:1000];
    }

    return self;
}

- (void) captureOutput:(AVCaptureOutput *)captureOutput
  didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection *)connection
{
    _last_input = [NSDate date];
    FileWrapper *wrapper = _output_wrapper;
    if (wrapper != nil) {
        CMBlockBufferRef block_buffer = CMSampleBufferGetDataBuffer(sampleBuffer);
        if (block_buffer == nil) {
            return;
        }
        size_t bb_len = CMBlockBufferGetDataLength(block_buffer);
        if (bb_len == 0) {
            return;
        }
        if (_output_data.length != bb_len) {
            _output_data.length = bb_len;
        }
        CMBlockBufferCopyDataBytes(block_buffer, 0, _output_data.length, _output_data.mutableBytes);

        wrapper->Parse_Buffer((const uint8_t*)_output_data.bytes, (size_t)_output_data.length);
    }
}

- (void) invalidateWrapper
{
    _output_wrapper = nil;
}

- (void) captureOutput:(AVCaptureOutput *)captureOutput
    didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection *)connection
{
    NSLog(@"Frame dropped.");
}
@end

// MARK: - AVFCtlExternalController (bridges C++ ControllerBaseWrapper to Obj-C)

@interface AVFCtlExternalController : NSObject
@property (assign,nonatomic) ControllerBaseWrapper *controller;
- (id) initWithController:(ControllerBaseWrapper*)extCtl;
- (NSString*) getStatus;
- (void) setPlaybackMode:(AVCaptureDeviceTransportControlsPlaybackMode)theMode speed:(AVCaptureDeviceTransportControlsSpeed) theSpeed;
- (AVCaptureDeviceTransportControlsPlaybackMode) getMode;
- (AVCaptureDeviceTransportControlsSpeed) getSpeed;
@end

@implementation AVFCtlExternalController

- (id) initWithController:(ControllerBaseWrapper*)extCtl
{
    self = [super init];

    if (self)
        _controller = extCtl;

    return self;
}

- (NSString*) getStatus
{
    if (_controller)
        return [NSString stringWithUTF8String:_controller->GetStatus().c_str()];

    return @"unknown";
}

- (void) setPlaybackMode:(AVCaptureDeviceTransportControlsPlaybackMode)theMode speed:(AVCaptureDeviceTransportControlsSpeed) theSpeed;
{
    if (_controller)
        _controller->SetPlaybackMode((playback_mode)theMode, (float)theSpeed);
}

- (AVCaptureDeviceTransportControlsPlaybackMode) getMode
{
    if (_controller)
        return (AVCaptureDeviceTransportControlsPlaybackMode)_controller->GetMode();

    return AVCaptureDeviceTransportControlsNotPlayingMode;
}

- (AVCaptureDeviceTransportControlsSpeed) getSpeed
{
    if (_controller)
        return (AVCaptureDeviceTransportControlsSpeed)_controller->GetSpeed();

    return (AVCaptureDeviceTransportControlsSpeed)0.0f;
}
@end

// MARK: - C++ AVFCtlWrapper implementation

const string AVFCtlWrapper::Interface = "DV";

AVFCtlWrapper::AVFCtlWrapper(size_t DeviceIndex, ControllerBaseWrapper* ExtCtl) : ExtCtl(ExtCtl)
{
    AVFCtlExternalController* ExternalController = nil;
    if (ExtCtl)
        ExternalController = [[AVFCtlExternalController alloc] initWithController:ExtCtl];

    Ctl = (void*)[[AVFCtl alloc] initWithDeviceIndex:DeviceIndex controller:ExternalController];
}

AVFCtlWrapper::AVFCtlWrapper(string DeviceID, ControllerBaseWrapper* ExtCtl) : ExtCtl(ExtCtl)
{
    AVFCtlExternalController* ExternalController = nil;
    if (ExtCtl)
        ExternalController = [[AVFCtlExternalController alloc] initWithController:ExtCtl];

    Ctl = (void*)[[AVFCtl alloc] initWithDeviceID:[NSString stringWithUTF8String:DeviceID.c_str()] controller:ExternalController];
}

AVFCtlWrapper::~AVFCtlWrapper()
{
    [(id)Ctl release];
}

size_t AVFCtlWrapper::GetDeviceCount()
{
    return (size_t)[AVFCtl getDeviceCount];
}

string AVFCtlWrapper::GetDeviceName(size_t DeviceIndex)
{
    return string([[AVFCtl getDeviceName:DeviceIndex] UTF8String]);
}

string AVFCtlWrapper::GetDeviceName(const std::string& DeviceID)
{
    NSInteger DeviceIndex = GetDeviceIndex(DeviceID);
    if (DeviceIndex < 0)
        return string();

    return string([[AVFCtl getDeviceName:DeviceIndex] UTF8String]);
}

string AVFCtlWrapper::GetDeviceID(size_t DeviceIndex)
{
    return string([[AVFCtl getDeviceID:DeviceIndex] UTF8String]);
}

size_t AVFCtlWrapper::GetDeviceIndex(const string& DeviceID)
{
    NSInteger index = [AVFCtl getDeviceIndex:[NSString stringWithUTF8String:DeviceID.c_str()]];
    if (index < 0)
        return (size_t)-1;

    return (size_t)index;
}

string AVFCtlWrapper::GetStatus()
{
    return string([[(id)Ctl getStatus] UTF8String]);
}

void AVFCtlWrapper::CreateCaptureSession(FileWrapper* Wrapper)
{
    AVFCtlBufferReceiver *receiver = [[AVFCtlBufferReceiver alloc] initWithFileWrapper:Wrapper];
    [(id)Ctl createCaptureSession:receiver];
}

void AVFCtlWrapper::StartCaptureSession()
{
    [(id)Ctl startCaptureSession];
}

void AVFCtlWrapper::StopCaptureSession()
{
    [(id)Ctl stopCaptureSession];
}

void AVFCtlWrapper::SetPlaybackMode(playback_mode Mode, float Speed)
{
    [(id)Ctl setPlaybackMode:(AVCaptureDeviceTransportControlsPlaybackMode)Mode speed:Speed];
}

float AVFCtlWrapper::GetSpeed()
{
    return (float)[(id)Ctl getSpeed];
}

playback_mode AVFCtlWrapper::GetMode()
{
    return (playback_mode)[(id)Ctl getMode];
}

bool AVFCtlWrapper::WaitForSessionEnd(uint64_t Timeout)
{
    return (bool)[(id)Ctl waitForSessionEnd: Timeout];
}

device_capabilities AVFCtlWrapper::GetCapabilities()
{
    device_capabilities Caps;
    Caps.Interface = Interface;

    // Device identification from IOKit FireWire registry
    NSString* vendor = [(id)Ctl getDeviceVendor];
    NSString* model = [(id)Ctl getDeviceModel];
    if (vendor) Caps.Vendor = string([vendor UTF8String]);
    if (model) Caps.Model = string([model UTF8String]);

    // Use AV/C SPECIFIC_INQUIRY to probe transport capabilities
    Caps.CanPlay = [(id)Ctl probeAvcCommand:0xC3 operand:0x38];
    if (Caps.CanPlay) Caps.SupportedForwardSpeeds.push_back(1.0f);
    Caps.CanPause = [(id)Ctl probeAvcCommand:0xC3 operand:0x7D];

    if ([(id)Ctl probeAvcCommand:0xC3 operand:0x32])
    {
        Caps.CanSlowForward = true;
        Caps.SupportedForwardSpeeds.push_back(0.5f);
    }
    if ([(id)Ctl probeAvcCommand:0xC3 operand:0x3E])
    {
        Caps.CanFastForward = true;
        Caps.SupportedForwardSpeeds.push_back(2.0f);
    }

    Caps.CanReverse = [(id)Ctl probeAvcCommand:0xC3 operand:0x48];
    if (Caps.CanReverse) Caps.SupportedReverseSpeeds.push_back(-1.0f);

    if ([(id)Ctl probeAvcCommand:0xC3 operand:0x4E])
    {
        Caps.CanSlowReverse = true;
        Caps.SupportedReverseSpeeds.push_back(-0.5f);
    }
    if ([(id)Ctl probeAvcCommand:0xC3 operand:0x42])
    {
        Caps.CanFastReverse = true;
        Caps.SupportedReverseSpeeds.push_back(-2.0f);
    }

    Caps.CanWind = [(id)Ctl probeAvcCommand:0xC4 operand:0x65];

    if ([(id)Ctl probeAvcCommand:0xC3 operand:0x3F] &&
        [(id)Ctl probeAvcCommand:0xC3 operand:0x41])
        Caps.CanShuttle = true;

    Caps.CanOutputReverse = Caps.CanReverse;
    Caps.CanRecord = [(id)Ctl probeAvcGeneral:0xC2];

    Caps.OutputSignalMode = [(id)Ctl queryAvcStatus:0x78 operand:0xFF];
    if (Caps.OutputSignalMode != 0xFF)
        Caps.SignalMode = device_capabilities::SignalModeName(Caps.OutputSignalMode);

    Caps.CassetteType = [(id)Ctl queryAvcStatus:0xDA operand:0xFF];
    if (Caps.CassetteType != 0xFF)
        Caps.HasTape = (Caps.CassetteType != 0x60);

    Caps.Probed = true;
    return Caps;
}
