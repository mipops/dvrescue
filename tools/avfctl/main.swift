/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

import AVFoundation
import Foundation

let avfctlVersion = "24.07"

func printHelp(full: Bool) {
    var output = ""
    output += "avfctl - control and capture from DV tape players via AVFoundation\n"
    output += "version \(avfctlVersion)\n\n"
    output += "Usage: avfctl [Options...] [OutputFile]\n\n"

    if full {
        output += "Options:\n"
        output += "-h\n"
        output += "Show this help and exit.\n\n"
        output += "-list_devices\n"
        output += "List detected devices and their indices.\n\n"
        output += "-device <arg>\n"
        output += "Specify the device to send commands to. <arg> is required and is the index\n"
        output += "or the id of the device as shown in -list_devices.\n"
        output += "If not specified, device with the index \"0\" is used by default.\n\n"
        output += "-status\n"
        output += "Show the current status of the device.\n\n"
        output += "-foreground\n"
        output += "Stay at foreground during play, ff or rew operation.\n\n"
        output += "-cmd <arg>\n"
        output += "  play      Set speed to 1.0 and mode to play.\n"
        output += "  srew      Set speed to -1.0 and mode to play.\n"
        output += "  stop      Set speed to 0.0 and mode to no-play.\n"
        output += "  rew       Set speed to -2.0 and mode to no-play.\n"
        output += "  ff        Set speed to 2.0 and mode to no-play.\n"
        output += "  capture   Set speed to 1.0 and mode to play and capture all raw-data\n"
        output += "            and save to [OutputFile].\n\n"
        output += "If [OutputFile] is not specified, data will be written to ./out.dv.\n"
        output += "If [OutputFile] is \"-\", data will be written to standard output.\n"
    } else {
        output += "\"avfctl -h\" for displaying more information.\n"
    }

    FileHandle.standardError.write(output.data(using: .utf8)!)
}

func parseDeviceIndex(_ str: String) -> Int? {
    if let value = Int(str) {
        return value
    }
    return nil
}

// MARK: - Main

var deviceID: String?
var foreground = false
var outputFilename = "out.dv"
var listDevices = false
var printStatus = false
var command: String?

let args = CommandLine.arguments
var pos = 1
while pos < args.count {
    let arg = args[pos]
    switch arg {
    case "-list_devices":
        listDevices = true
    case "-foreground":
        foreground = true
    case "-status":
        printStatus = true
    case "-device":
        pos += 1
        guard pos < args.count else {
            NSLog("No device given.")
            exit(1)
        }
        if let idx = parseDeviceIndex(args[pos]) {
            let id = AVFCtl.getDeviceID(idx)
            if id.isEmpty {
                NSLog("Invalid device index given.")
                exit(1)
            }
            deviceID = id
        } else {
            let index = AVFCtl.getDeviceIndex(args[pos])
            if index < 0 {
                NSLog("Invalid device id given.")
                exit(1)
            }
            deviceID = args[pos]
        }
    case "-cmd":
        pos += 1
        guard pos < args.count else {
            NSLog("No command given.")
            exit(1)
        }
        command = args[pos]
    case "-h":
        printHelp(full: true)
        exit(0)
    default:
        if pos == args.count - 1 {
            outputFilename = arg
        }
    }
    pos += 1
}

let hasArgs = listDevices || printStatus || command != nil
if !hasArgs {
    printHelp(full: false)
    exit(1)
}

// Bail out if no devices are found
if AVFCtl.getDeviceCount() == 0 {
    NSLog("No devices found.")
    exit(1)
}

// List devices if requested
if listDevices {
    NSLog("Devices:")
    for idx in 0..<AVFCtl.getDeviceCount() {
        NSLog("[\(idx) - \(AVFCtl.getDeviceID(idx))] \(AVFCtl.getDeviceName(idx))")
    }
}

// Use first device by default
if deviceID == nil {
    deviceID = AVFCtl.getDeviceID(0)
}

let deviceIndex = AVFCtl.getDeviceIndex(deviceID!)
let deviceName = AVFCtl.getDeviceName(deviceIndex)

// Check transport control support
if !AVFCtl.isTransportControlsSupported(deviceIndex) {
    NSLog("Transport Controls not supported for device [\(deviceID!)] \(deviceName).")
    exit(1)
}

guard let avfctl = AVFCtl(deviceID: deviceID!, controller: nil) else {
    NSLog("Error creating AVFCtl instance.")
    exit(1)
}

// Print status if requested
if printStatus {
    let nullReceiver = AVFCtlFileReceiver(outputFileName: "/dev/null")
    avfctl.createCaptureSession(nullReceiver)
    RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
    NSLog("Device [\(deviceID!)] \(deviceName) status: \(avfctl.getStatus())")
}

// Show device status changes
avfctl.logChanges = true

// Execute command
if let cmd = command {
    switch cmd.lowercased() {
    case "play":
        if foreground {
            let nullReceiver = AVFCtlFileReceiver(outputFileName: "/dev/null")
            avfctl.createCaptureSession(nullReceiver)
            avfctl.startCaptureSession()
        }
        avfctl.setPlaybackMode(.playingMode, speed: 1.0)
        if foreground {
            _ = avfctl.waitForSessionEnd(0)
            avfctl.stopCaptureSession()
        }

    case "stop":
        avfctl.setPlaybackMode(.notPlayingMode, speed: 0.0)

    case "rew":
        if foreground {
            let nullReceiver = AVFCtlFileReceiver(outputFileName: "/dev/null")
            avfctl.createCaptureSession(nullReceiver)
            avfctl.startCaptureSession()
        }
        avfctl.setPlaybackMode(.notPlayingMode, speed: -2.0)
        if foreground {
            _ = avfctl.waitForSessionEnd(0)
            avfctl.stopCaptureSession()
        }

    case "srew":
        if foreground {
            let nullReceiver = AVFCtlFileReceiver(outputFileName: "/dev/null")
            avfctl.createCaptureSession(nullReceiver)
            avfctl.startCaptureSession()
        }
        avfctl.setPlaybackMode(.playingMode, speed: -1.0)
        if foreground {
            _ = avfctl.waitForSessionEnd(0)
            avfctl.stopCaptureSession()
        }

    case "ff":
        if foreground {
            let nullReceiver = AVFCtlFileReceiver(outputFileName: "/dev/null")
            avfctl.createCaptureSession(nullReceiver)
            avfctl.startCaptureSession()
        }
        avfctl.setPlaybackMode(.notPlayingMode, speed: 2.0)
        if foreground {
            _ = avfctl.waitForSessionEnd(0)
            avfctl.stopCaptureSession()
        }

    case "capture":
        let receiver = AVFCtlFileReceiver(outputFileName: outputFilename)
        avfctl.createCaptureSession(receiver)
        avfctl.startCaptureSession()
        avfctl.setPlaybackMode(.playingMode, speed: 1.0)
        _ = avfctl.waitForSessionEnd(0)
        avfctl.stopCaptureSession()

    default:
        NSLog("Invalid command given.")
    }
}

exit(0)
