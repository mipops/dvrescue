/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once

#import <AVFoundation/AVFoundation.h>

//***************************************************************************
// Protocol ReceiverTimer
//***************************************************************************
@protocol ReceiverTimer
 - (NSDate*) lastInput;
 - (void) setLastInput: (NSDate*) toDate;
@end

//***************************************************************************
// Interface AVFCtlFileReceiver
//***************************************************************************

@interface AVFCtlFileReceiver : NSObject <ReceiverTimer>
@property (nonatomic, retain) NSMutableData *output_data;
@property (nonatomic, retain) NSFileHandle *output_file;
@property (atomic, strong) NSDate *last_input;

- (NSDate*) lastInput;
- (void) setLastInput: (NSDate*) toDate;
- (id) initWithOutputFileName:(NSString*)theFileName;
- (void) captureOutput:(AVCaptureOutput*)captureOutput
  didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection*)connection;
- (void) captureOutput:(AVCaptureOutput*)captureOutput
    didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection*)connection;
@end

//***************************************************************************
// Interface AVFCtl
//***************************************************************************

@interface AVFCtl : NSObject {
    id <ReceiverTimer> receiverInstance;
    id                 externalController;
}

@property (nonatomic, retain) AVCaptureDevice *device;
@property (nonatomic, retain) AVCaptureSession *session;
@property (nonatomic, retain) AVCaptureVideoDataOutput *output;
@property AVCaptureDeviceTransportControlsPlaybackMode old_mode;
@property AVCaptureDeviceTransportControlsSpeed old_speed;
@property BOOL log_changes;

+ (NSUInteger) getDeviceCount;
+ (NSString*) getDeviceName:(NSUInteger) index;
+ (NSString*) getDeviceID:(NSUInteger) index;
+ (NSInteger) getDeviceIndex:(NSString*) uniqueID;
+ (BOOL) isTransportControlsSupported:(NSUInteger) index;
- (id) initWithDeviceIndex:(NSUInteger) index controller:(id) extCtl;
- (id) initWithDeviceID:(NSString*) uniqueID controller:(id) extCtl;
- (void) dealloc;
- (NSString*) getStatus;
- (void) createCaptureSession:(id) receiver;
- (void) startCaptureSession;
- (void) stopCaptureSession;
- (void) setPlaybackMode:(AVCaptureDeviceTransportControlsPlaybackMode)theMode speed:(AVCaptureDeviceTransportControlsSpeed) theSpeed;
- (AVCaptureDeviceTransportControlsPlaybackMode) getMode;
- (AVCaptureDeviceTransportControlsSpeed) getSpeed;
- (BOOL) waitForSessionEnd:(NSUInteger) timeout;
@end
