/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

#import <avfctl.h>

int get_device_idx(NSString *str)
{
       NSScanner *scan = [NSScanner scannerWithString:str];
       int device_idx;
       [scan scanInt:&device_idx];
       if ([scan isAtEnd])
           return device_idx;
       else
           return -1;
}

int main(int argc, char *argv[])
{
    int device_idx = -1;
    NSUserDefaults *args = [NSUserDefaults standardUserDefaults];
    AVCaptureDevice *device = nil;

    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeMuxed];

    // bail out if no devices are found
    if ([devices count] == 0) {
        NSLog(@"No devices found.");
        return 1;
    }

    // list devices upfront if requested
    if ([args objectForKey:@"list_devices"]) {
        NSLog(@"Devices:");
        for (AVCaptureDevice *device in devices) {
            NSLog(@"[%ld] %s", [devices indexOfObject:device], [[device localizedName] UTF8String]);
        }
    }

    // get device index given at cmd line
    if ([args objectForKey:@"device"]) {
        device_idx = get_device_idx([args stringForKey:@"device"]);
        if (device_idx < 0 || device_idx >= [devices count]) {
            // error out if no valid device number could be identified from cmd line
            NSLog(@"Invalid device index given.");
            return 1;
        }
    }

    // error out if no device could be identified from cmd line
    if (device_idx < 0) {
        NSLog(@"No device index given.");
        return 1;
    }

    @try {
        device = devices[device_idx];
    }
    @catch (NSException *e) {
        NSLog(@"Error: %@", e);
        return 1;
    }

    // check if device supports transport control
    if (![device transportControlsSupported]) {
        NSLog(@"Transport Controls not supported for device [%d] %s.", device_idx, [[device localizedName] UTF8String]);
        return 1;
    }


    // create AVFCTL with device
    AVFCtl *avfctl = [[AVFCtl alloc] initWithDevice: device];

    // execute command
    if ([args objectForKey:@"cmd"]) {
        NSString *cmd = [args stringForKey:@"cmd"];

        if ([cmd isEqualToString:@"PLAY"] ||
            [cmd isEqualToString:@"play"]) {
            // do PLAY
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsPlayingMode speed:1.0f];
        } else if ([cmd isEqualToString:@"STOP"] ||
                   [cmd isEqualToString:@"stop"]) {
            // do STOP
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsNotPlayingMode speed:0.0f];
        } else if ([cmd isEqualToString:@"REW"] ||
                   [cmd isEqualToString:@"rew"]) {
            // do REW
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsNotPlayingMode speed:-2.0f];
        } else if ([cmd isEqualToString:@"FF"] ||
                   [cmd isEqualToString:@"ff"]) {
            // do FF
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsNotPlayingMode speed:2.0f];
        } else if ([cmd isEqualToString:@"CAPTURE"] ||
                   [cmd isEqualToString:@"capture"]) {
            // do CAPTURE
            [avfctl createCaptureSessionWithOutputFileName:@"out.dv"];
            [avfctl startCaptureSession];
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsPlayingMode speed:1.0f];

            // block as long as the capture session is running
            // terminates if playback mode changes to NotPlaying (observed in avfctl)
            // busy waiting, do not use in production!
            while([avfctl.session isRunning]) {
            }

            [avfctl stopCaptureSession]; // redundant for internal errors in the session
        } else {
            NSLog(@"No command given.");
        }

    }
    return 0;
}


// legacy main loop for capturing driven by keyboard input
/*
            NSFileHandle *handle = [NSFileHandle fileHandleWithStandardInput];
            NSData *data = nil;
            while ((data = handle.availableData)) {
                NSString *input = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
                NSCharacterSet *set = [NSCharacterSet newlineCharacterSet];
                NSString *userInput = [input stringByTrimmingCharactersInSet:set];

                NSLog(@"Read: %@", userInput);

                if ([userInput isEqualToString:@"q"]) {
                    break;
                }
                if ([userInput isEqualToString:@"startCapture"]) {
                    NSLog(@"Start");
                    [avfctl createCaptureSessionWithOutputFileName:@"out.dv"];
                    [avfctl startCaptureSession];
                }
                if ([userInput isEqualToString:@"stopCapture"]) {
                    NSLog(@"Stop");
                    [avfctl stopCaptureSession];
                }
            }
*/

