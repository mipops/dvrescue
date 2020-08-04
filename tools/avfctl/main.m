/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

#import <avfctl.h>

void help(BOOL full)
{
    NSString* version = @"0.20.06";
    NSMutableString *output = [[NSMutableString alloc] init];

    [output appendString:@"avfctl - control and capture from DV tape players via AVFoundation\n"];
    [output appendFormat:@"version %@\n\n", version];
    [output appendString:@"Usage: avfctl [Options...] [OutputFile]\n\n"];

    if (full) {
        [output appendString:@"Options:\n"];
        [output appendString:@"-h\n"];
        [output appendString:@"Show this help and exit.\n\n"];
        [output appendString:@"-list_devices\n"];
        [output appendString:@"List detected devices and their indices.\n\n"];
        [output appendString:@"-device <arg>\n"];
        [output appendString:@"Specify the device to send commands to. <arg> is required and is the index of the device as shown in -list_devices.\n"];
        [output appendString:@"If not specified, device with the index \"0\" is used by default.\n\n"];
        [output appendString:@"-status\n"];
        [output appendString:@"Show the current status of the device.\n\n"];
        [output appendString:@"-foreground\n"];
        [output appendString:@"Stay at foreground during play, ff or rew operation.\n\n"];
        [output appendString:@"-cmd <arg>\n"];
        [output appendString:@"  play      Set speed to 1.0 and mode to play.\n"];
        [output appendString:@"  stop      Set speed to 0.0 and mode to no-play.\n"];
        [output appendString:@"  rew       Set speed to -2.0 and mode to no-play.\n"];
        [output appendString:@"  ff        Set speed to 2.0 and mode to no-play.\n"];
        [output appendString:@"  capture   Set speed to 1.0 and mode to play and capture all raw-data and save to [OutputFile].\n\n"];
        [output appendString:@"If [OutputFile] is not specified, data will be written to ./out.dv.\n"];
        [output appendString:@"If [OutputFile] is \"-\", data will be written to standard output.\n"];
    } else {
        [output appendString:@"\"avfctl -h\" for displaying more information.\n"];
    }

    [[NSFileHandle fileHandleWithStandardError] writeData:[output dataUsingEncoding:NSUTF8StringEncoding]];
}

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
    int device_idx = 0;
    bool foreground = false;
    NSString *output_filename = @"out.dv";

    AVCaptureDevice *device = nil;

    NSMutableDictionary *args = [[NSMutableDictionary alloc] init];

    NSArray *arguments = [[NSProcessInfo processInfo] arguments];
    for (int pos = 0; pos < [arguments count]; pos++) {
        if ([[arguments objectAtIndex:pos] isEqualTo: @"-list_devices"]) {
            [args setObject: @YES forKey: @"list_devices"];
        } else if ([[arguments objectAtIndex:pos] isEqualTo: @"-foreground"]) {
            foreground = true;
        } else if ([[arguments objectAtIndex:pos] isEqualTo: @"-status"]) {
            [args setObject: @YES forKey: @"print_status"];
        } else if ([[arguments objectAtIndex:pos] isEqualTo: @"-device"]) {
            if (++pos < [arguments count]) {
                device_idx = get_device_idx([arguments objectAtIndex:pos]);
            } else {
                NSLog(@"No device given.");
                return 1;
            }
        } else if ([[arguments objectAtIndex:pos] isEqualTo: @"-cmd"]) {
            if (++pos < [arguments count]) {
                [args setObject: [arguments objectAtIndex:pos] forKey: @"cmd"];
            } else {
                NSLog(@"No command given.");
                return 1;
            }
        } else if ([[arguments objectAtIndex:pos] isEqualTo: @"-h"]) {
            help(YES);
            return 0;
        } else if (pos == [arguments count] - 1) { // Last argument is the output filename
            output_filename = [arguments objectAtIndex:pos];
        }
    }

    if ([args count] == 0) {
        help(NO);
        return 1;
    }

    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeMuxed];

    // bail out if no devices are found
    if ([devices count] == 0) {
        NSLog(@"No devices found.");
        return 1;
    }

    // list devices upfront if requested
    if ([[args objectForKey:@"list_devices"] isEqualTo: @YES]) {
        NSLog(@"Devices:");
        for (AVCaptureDevice *device in devices) {
            NSLog(@"[%ld] %s", [devices indexOfObject:device], [[device localizedName] UTF8String]);
        }
    }

    // check device index
    if (device_idx < 0 || device_idx >= [devices count]) {
        // error out if no valid device number could be identified from cmd line
        NSLog(@"Invalid device index given.");
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

    // Print status if requested
    if ([[args objectForKey:@"print_status"] isEqualTo: @YES]) {
        avfctl.status_mode = YES;

        [avfctl createCaptureSessionWithOutputFileName:@"/dev/null"];
        // give time for the driver to retrieves status from the device
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];
        NSLog(@"Device [%d] %@ status: %@", device_idx, [device localizedName], [avfctl getStatus]);
    }

    // execute command
    if ([args objectForKey:@"cmd"]) {
        NSString *cmd = [args objectForKey:@"cmd"];

        if ([cmd isEqualToString:@"PLAY"] ||
            [cmd isEqualToString:@"play"]) {
            // do PLAY
            if (foreground) {
                [avfctl createCaptureSessionWithOutputFileName:@"/dev/null"];
                [avfctl startCaptureSession];
            }
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsPlayingMode speed:1.0f];
            if (foreground) {
                [avfctl waitForSessionEnd];
                [avfctl stopCaptureSession];
            }
        } else if ([cmd isEqualToString:@"STOP"] ||
                   [cmd isEqualToString:@"stop"]) {
            // do STOP
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsNotPlayingMode speed:0.0f];
        } else if ([cmd isEqualToString:@"REW"] ||
                   [cmd isEqualToString:@"rew"]) {
            // do REW
            if (foreground) {
                [avfctl createCaptureSessionWithOutputFileName:@"/dev/null"];
                [avfctl startCaptureSession];
            }
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsNotPlayingMode speed:-2.0f];
            if (foreground) {
                [avfctl waitForSessionEnd];
                [avfctl stopCaptureSession];
            }
        } else if ([cmd isEqualToString:@"FF"] ||
                   [cmd isEqualToString:@"ff"]) {
            // do FF
            if (foreground) {
                [avfctl createCaptureSessionWithOutputFileName:@"/dev/null"];
                [avfctl startCaptureSession];
            }
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsNotPlayingMode speed:2.0f];
            if (foreground) {
                [avfctl waitForSessionEnd];
                [avfctl stopCaptureSession];
            }
        } else if ([cmd isEqualToString:@"CAPTURE"] ||
                   [cmd isEqualToString:@"capture"]) {
            // do CAPTURE
            [avfctl createCaptureSessionWithOutputFileName:output_filename];
            [avfctl startCaptureSession];
            [avfctl setPlaybackMode:AVCaptureDeviceTransportControlsPlayingMode speed:1.0f];

            [avfctl waitForSessionEnd];

            [avfctl stopCaptureSession]; // redundant for internal errors in the session
        } else {
            NSLog(@"Invalid command given.");
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

