/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

#include "Common/AvfCtlWrapper.h"
#import "Common/AvfCtl.h"

using namespace std;

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
    if (_output_wrapper != nil) {
        CMBlockBufferRef block_buffer = CMSampleBufferGetDataBuffer(sampleBuffer); // raw, DV data only
        size_t bb_len = CMBlockBufferGetDataLength(block_buffer);
        if (_output_data.length != bb_len) {
            _output_data.length = bb_len;
        }
        CMBlockBufferCopyDataBytes(block_buffer, 0, _output_data.length, _output_data.mutableBytes);
        
        _output_wrapper->Parse_Buffer((const uint8_t*)_output_data.bytes, (size_t)_output_data.length);
    }
}

- (void) captureOutput:(AVCaptureOutput *)captureOutput
    didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection *)connection
{
    NSLog(@"Frame dropped.");
}
@end

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
