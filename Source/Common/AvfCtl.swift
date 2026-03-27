/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

import AVFoundation
import Foundation
import IOKit
import IOKit.avc
import IOKit.pwr_mgt

// MARK: - AV/C Protocol Constants

private let VCR_CTL: UInt8                    = 0x00
private let VCR_0: UInt8                      = 0x20

private let VCR_CMD_PLAY: UInt8               = 0xC3
private let VCR_CMD_WIND: UInt8               = 0xC4

private let VCR_OPE_PLAY_FORWARD: UInt8       = 0x38
private let VCR_OPE_PLAY_FORWARD_PAUSE: UInt8 = 0x7D
private let VCR_OPE_PLAY_REVERSE: UInt8       = 0x48
private let VCR_OPE_PLAY_REVERSE_PAUSE: UInt8 = 0x6D

private let VCR_OPE_WIND_STOP: UInt8          = 0x60
private let VCR_OPE_WIND_REWIND: UInt8        = 0x65
private let VCR_OPE_WIND_FAST_FORWARD: UInt8  = 0x75

private let VCR_SPD_X1: UInt8 = 0x01
private let VCR_SPD_X2: UInt8 = 0x02
private let VCR_SPD_X3: UInt8 = 0x03
private let VCR_SPD_X4: UInt8 = 0x04
private let VCR_SPD_X5: UInt8 = 0x05
private let VCR_SPD_X6: UInt8 = 0x06
private let VCR_SPD_X7: UInt8 = 0x07

// MARK: - ReceiverTimer Protocol

@objc public protocol ReceiverTimer {
    func lastInput() -> Date?
    func setLastInput(_ toDate: Date)
}

// MARK: - AVFCtlFileReceiver

@objc public class AVFCtlFileReceiver: NSObject, ReceiverTimer, AVCaptureVideoDataOutputSampleBufferDelegate {
    @objc public var outputData: NSMutableData
    @objc public var outputFile: FileHandle?
    private var _lastInput: Date?
    private let lastInputLock = NSLock()

    @objc public func lastInput() -> Date? {
        lastInputLock.lock()
        defer { lastInputLock.unlock() }
        return _lastInput
    }

    @objc public func setLastInput(_ toDate: Date) {
        lastInputLock.lock()
        defer { lastInputLock.unlock() }
        _lastInput = toDate
    }

    @objc public init(outputFileName: String) {
        outputData = NSMutableData(length: 1000)!
        super.init()

        if outputFileName == "-" {
            outputFile = FileHandle.standardOutput
        } else {
            FileManager.default.createFile(atPath: outputFileName, contents: nil, attributes: nil)
            outputFile = FileHandle(forWritingAtPath: outputFileName)
        }
    }

    @objc public func captureOutput(_ output: AVCaptureOutput,
                                    didOutput sampleBuffer: CMSampleBuffer,
                                    from connection: AVCaptureConnection) {
        setLastInput(Date())
        guard let file = outputFile else { return }

        guard let blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer) else { return }
        let length = CMBlockBufferGetDataLength(blockBuffer)
        if outputData.length != length {
            outputData.length = length
        }
        CMBlockBufferCopyDataBytes(blockBuffer, atOffset: 0, dataLength: length,
                                   destination: outputData.mutableBytes)
        file.write(outputData as Data)
    }

    @objc public func captureOutput(_ output: AVCaptureOutput,
                                    didDrop sampleBuffer: CMSampleBuffer,
                                    from connection: AVCaptureConnection) {
        NSLog("Frame dropped.")
    }
}

// MARK: - AVFCtl

@objc public class AVFCtl: NSObject {
    @objc public var device: AVCaptureDevice?
    @objc public var session: AVCaptureSession?
    @objc public var output: AVCaptureVideoDataOutput?
    @objc public var oldMode: AVCaptureDevice.TransportControlsPlaybackMode = .notPlayingMode
    @objc public var oldSpeed: AVCaptureDevice.TransportControlsSpeed = 0.0
    @objc public var logChanges: Bool = false

    private var receiverInstance: ReceiverTimer?
    private var externalController: AnyObject?
    private var avcDevice: UnsafeMutablePointer<UnsafeMutablePointer<IOFireWireAVCLibUnitInterface>?>?
    private var powerAssertionID: IOPMAssertionID = IOPMAssertionID(0)
    private var activityToken: NSObjectProtocol?

    // MARK: - Static Device Enumeration

    private static func muxedDevices() -> [AVCaptureDevice] {
        return AVCaptureDevice.devices(for: .muxed)
    }

    @objc public static func getDeviceCount() -> Int {
        return muxedDevices().count
    }

    @objc public static func getDeviceName(_ index: Int) -> String {
        let devices = muxedDevices()
        guard index < devices.count else { return "" }

        let device = devices[index]
        var name = device.localizedName
        var vendor = ""
        var model = ""

        var uniqueID = device.uniqueID
        if uniqueID.count > 2 && uniqueID.hasPrefix("0x") {
            uniqueID = String(uniqueID.dropFirst(2))
        }

        var iterator: io_iterator_t = 0
        let result = IOServiceGetMatchingServices(kIOMainPortDefault,
                                                   IOServiceNameMatching("IOFireWireDevice"),
                                                   &iterator)
        if result == KERN_SUCCESS {
            var service = IOIteratorNext(iterator)
            while service != 0 {
                var location = io_name_t()
                let locResult = withUnsafeMutablePointer(to: &location) { ptr in
                    ptr.withMemoryRebound(to: CChar.self, capacity: MemoryLayout<io_name_t>.size) { charPtr in
                        IORegistryEntryGetLocationInPlane(service, kIOServicePlane, charPtr)
                    }
                }
                let locationStr = withUnsafePointer(to: &location) { ptr in
                    ptr.withMemoryRebound(to: CChar.self, capacity: MemoryLayout<io_name_t>.size) { charPtr in
                        String(cString: charPtr)
                    }
                }

                if locResult == KERN_SUCCESS && locationStr == uniqueID {
                    var properties: Unmanaged<CFMutableDictionary>?
                    if IORegistryEntryCreateCFProperties(service, &properties, kCFAllocatorDefault, 0) == KERN_SUCCESS,
                       let dict = properties?.takeRetainedValue() as? [String: Any] {
                        if let v = dict["FireWire Vendor Name"] as? String, !v.isEmpty {
                            vendor = v
                        }
                        if let m = dict["FireWire Product Name"] as? String, !m.isEmpty {
                            model = m
                        }
                    }
                    IOObjectRelease(service)
                    break
                }
                IOObjectRelease(service)
                service = IOIteratorNext(iterator)
            }
            IOObjectRelease(iterator)
        }

        if !model.isEmpty {
            if !vendor.isEmpty {
                name += " (\(vendor) \(model))"
            } else {
                name += " (\(model))"
            }
        }

        return name
    }

    @objc public static func getDeviceID(_ index: Int) -> String {
        let devices = muxedDevices()
        guard index < devices.count else { return "" }
        return devices[index].uniqueID
    }

    @objc public static func getDeviceIndex(_ uniqueID: String) -> Int {
        let devices = muxedDevices()
        for (i, dev) in devices.enumerated() {
            if dev.uniqueID == uniqueID {
                return i
            }
        }
        return -1
    }

    @objc public static func isTransportControlsSupported(_ index: Int) -> Bool {
        let devices = muxedDevices()
        guard index < devices.count else { return false }
        return devices[index].transportControlsSupported
    }

    // MARK: - AVC Device Setup

    private func setupAVCDevice() {
        guard let device = device else { return }

        var uniqueIDValue: UInt64 = 0
        let scanner = Scanner(string: device.uniqueID)
        if scanner.scanHexInt64(&uniqueIDValue), uniqueIDValue == 0 { return }

        let matchingDict = IOServiceMatching("IOFireWireAVCUnit") as NSMutableDictionary
        let uniqueIDRef = CFNumberCreate(kCFAllocatorDefault, .sInt64Type, &uniqueIDValue)!
        matchingDict[kIOPropertyMatchKey as String] = nil // clear default
        matchingDict.setValue(uniqueIDRef, forKey: "GUID")
        let service = IOServiceGetMatchingService(kIOMainPortDefault,
                                                   matchingDict as CFDictionary)
        guard service != 0 else { return }

        var plugInInterface: UnsafeMutablePointer<UnsafeMutablePointer<IOCFPlugInInterface>?>?
        var score: Int32 = 0
        let ioResult = IOCreatePlugInInterfaceForService(
            service,
            kIOFireWireAVCLibUnitTypeID,
            kIOCFPlugInInterfaceID,
            &plugInInterface,
            &score
        )

        if ioResult == S_OK, let plugIn = plugInInterface?.pointee?.pointee {
            var avcRef: LPVOID?
            let queryResult = plugIn.QueryInterface(
                plugInInterface,
                CFUUIDGetUUIDBytes(kIOFireWireAVCLibUnitInterfaceID),
                &avcRef
            )
            if queryResult == S_OK, let avc = avcRef {
                avcDevice = avc.assumingMemoryBound(
                    to: UnsafeMutablePointer<IOFireWireAVCLibUnitInterface>?.self
                )
            }
            plugIn.Release(plugInInterface)
        }
        IOObjectRelease(service)
    }

    // MARK: - Initializers

    @objc public init?(deviceIndex index: Int, controller extCtl: AnyObject?) {
        let devices = AVFCtl.muxedDevices()
        guard index < devices.count else { return nil }

        super.init()
        device = devices[index]
        oldMode = device!.transportControlsPlaybackMode
        oldSpeed = device!.transportControlsSpeed
        logChanges = false
        externalController = extCtl

        device!.addObserver(self, forKeyPath: "transportControlsPlaybackMode",
                            options: .new, context: nil)
        device!.addObserver(self, forKeyPath: "transportControlsSpeed",
                            options: .new, context: nil)

        if externalController == nil {
            setupAVCDevice()
        }

        if avcDevice == nil || avcDevice?.pointee == nil {
            NSLog("Warning could not setup AVC device, falling back to known buggy AVFoundation transport controls")
        }
    }

    @objc public init?(deviceID uniqueID: String, controller extCtl: AnyObject?) {
        super.init()

        let devices = AVFCtl.muxedDevices()
        for dev in devices {
            if dev.uniqueID == uniqueID {
                device = dev
                break
            }
        }
        guard device != nil else { return nil }

        oldMode = device!.transportControlsPlaybackMode
        oldSpeed = device!.transportControlsSpeed
        logChanges = false
        externalController = extCtl

        device!.addObserver(self, forKeyPath: "transportControlsPlaybackMode",
                            options: .new, context: nil)
        device!.addObserver(self, forKeyPath: "transportControlsSpeed",
                            options: .new, context: nil)

        if externalController == nil {
            setupAVCDevice()
        }

        if avcDevice == nil || avcDevice?.pointee == nil {
            NSLog("Warning could not setup AVC device, falling back to known buggy AVFoundation transport controls")
        }
    }

    deinit {
        allowSleep() // Release power assertions if still held

        if let device = device {
            device.removeObserver(self, forKeyPath: "transportControlsPlaybackMode")
            device.removeObserver(self, forKeyPath: "transportControlsSpeed")
        }

        if let avc = avcDevice, let iface = avc.pointee {
            iface.pointee.Release(avc)
        }
    }

    // MARK: - KVO

    public override func observeValue(forKeyPath keyPath: String?, of object: Any?,
                                      change: [NSKeyValueChangeKey: Any]?, context: UnsafeMutableRawPointer?) {
        guard context == nil else {
            super.observeValue(forKeyPath: keyPath, of: object, change: change, context: context)
            return
        }

        if keyPath == "transportControlsPlaybackMode" {
            if let modeValue = change?[.newKey] as? Int,
               let mode = AVCaptureDevice.TransportControlsPlaybackMode(rawValue: modeValue) {
                if oldMode != mode {
                    if logChanges {
                        NSLog("Mode changed: \(oldMode.rawValue) -> \(mode.rawValue)")
                    }
                    if mode == .notPlayingMode, let session = session, session.isRunning {
                        if logChanges {
                            NSLog("Stopping capturing.")
                        }
                        stopCaptureSession()
                    }
                    oldMode = mode
                }
            }
        } else if keyPath == "transportControlsSpeed" {
            if let speed = change?[.newKey] as? Float {
                if oldSpeed != speed {
                    if logChanges {
                        NSLog("Speed changed: \(oldSpeed) -> \(speed)")
                    }
                    oldSpeed = speed
                }
            }
        }
    }

    // MARK: - Status

    @objc public func getStatus() -> String {
        if let ctrl = externalController, ctrl.responds(to: #selector(getStatus)) {
            return ctrl.perform(#selector(getStatus))?.takeUnretainedValue() as? String ?? "unknown"
        }

        guard let device = device else { return "unknown" }
        let speed = device.transportControlsSpeed
        if speed == 0.0 { return "stopped" }
        if speed == 1.0 { return "playing" }
        if speed > 1.0 { return "fast-forwarding" }
        if speed < 0.0 { return "rewinding" }
        return "unknown"
    }

    // MARK: - Capture Session

    @objc public func createCaptureSession(_ receiver: AnyObject) {
        guard let device = device else { return }
        guard !device.isInUseByAnotherApplication else {
            NSLog("Error creating capture session: device already in use")
            return
        }

        do {
            session = AVCaptureSession()

            let deviceInput = try AVCaptureDeviceInput(device: device)
            guard session!.canAddInput(deviceInput) else {
                NSLog("Error adding input to session")
                return
            }
            session!.addInput(deviceInput)

            output = AVCaptureVideoDataOutput()
            output!.videoSettings = [:] // empty dict to receive raw data
            output!.alwaysDiscardsLateVideoFrames = false

            let queue = DispatchQueue(label: "avfctl_queue")
            output!.setSampleBufferDelegate(receiver as? AVCaptureVideoDataOutputSampleBufferDelegate,
                                            queue: queue)

            guard session!.canAddOutput(output!) else {
                NSLog("Error adding output to session")
                return
            }
            session!.addOutput(output!)
        } catch {
            NSLog("Error creating capture session: \(error)")
            return
        }

        receiverInstance = receiver as? ReceiverTimer
    }

    @objc public func startCaptureSession() {
        preventSleep()
        session?.startRunning()
    }

    @objc public func stopCaptureSession() {
        if let receiver = receiverInstance as? NSObject,
           receiver.responds(to: Selector(("invalidateWrapper"))) {
            receiver.perform(Selector(("invalidateWrapper")))
        }
        session?.stopRunning()
        allowSleep()
    }

    // MARK: - Power Management
    // Prevent display and system sleep during capture. macOS suspends GCD dispatch
    // queues servicing AVCaptureVideoDataOutput when the display sleeps, causing
    // frames to buffer in the kernel and flush all at once on wake — producing a
    // fast-forward effect in the captured file. Taking an IOPMAssertion for
    // PreventUserIdleDisplaySleep keeps the display (and thus the capture pipeline)
    // alive for the duration of the session.

    private func preventSleep() {
        // IOKit power assertion: prevents both display and system idle sleep
        let reason = "DVRescue: DV capture session active" as CFString
        let result = IOPMAssertionCreateWithName(
            kIOPMAssertionTypePreventUserIdleDisplaySleep as CFString,
            IOPMAssertionLevel(kIOPMAssertionLevelOn),
            reason,
            &powerAssertionID
        )
        if result != kIOReturnSuccess {
            NSLog("Warning: could not create power assertion to prevent display sleep (IOReturn %d)", result)
        }

        // NSProcessInfo activity: prevents App Nap and sudden termination
        activityToken = ProcessInfo.processInfo.beginActivity(
            options: [.userInitiated, .idleDisplaySleepDisabled, .idleSystemSleepDisabled],
            reason: "DV capture session active"
        )
    }

    private func allowSleep() {
        if powerAssertionID != IOPMAssertionID(0) {
            IOPMAssertionRelease(powerAssertionID)
            powerAssertionID = IOPMAssertionID(0)
        }

        if let token = activityToken {
            ProcessInfo.processInfo.endActivity(token)
            activityToken = nil
        }
    }

    // MARK: - Playback Mode

    @objc public func setPlaybackMode(_ theMode: AVCaptureDevice.TransportControlsPlaybackMode,
                                      speed theSpeed: Float) {
        if let ctrl = externalController,
           ctrl.responds(to: #selector(AVFCtl.setPlaybackMode(_:speed:))) {
            _ = ctrl.perform(#selector(AVFCtl.setPlaybackMode(_:speed:)),
                             with: NSNumber(value: theMode.rawValue),
                             with: NSNumber(value: theSpeed))
            return
        }

        if let avc = avcDevice, let iface = avc.pointee {
            var command: [UInt8] = [VCR_CTL, VCR_0, 0, 0]

            switch theMode {
            case .playingMode:
                command[2] = VCR_CMD_PLAY
                if theSpeed >= 2.0 {
                    command[3] = VCR_OPE_PLAY_FORWARD &+ VCR_SPD_X7
                } else if theSpeed > 1.0 {
                    command[3] = VCR_OPE_PLAY_FORWARD &+ VCR_SPD_X6
                } else if theSpeed == 1.0 {
                    command[3] = VCR_OPE_PLAY_FORWARD
                } else if theSpeed > 0.0 {
                    command[3] = VCR_OPE_PLAY_FORWARD &- VCR_SPD_X6
                } else if theSpeed == 0.0 {
                    command[3] = VCR_OPE_PLAY_FORWARD_PAUSE
                } else if theSpeed > -1.0 {
                    command[3] = VCR_OPE_PLAY_REVERSE &+ VCR_SPD_X6
                } else if theSpeed == -1.0 {
                    command[3] = VCR_OPE_PLAY_REVERSE
                } else if theSpeed > -2.0 {
                    command[3] = VCR_OPE_PLAY_REVERSE &- VCR_SPD_X6
                } else {
                    command[3] = VCR_OPE_PLAY_REVERSE &- VCR_SPD_X7
                }
            case .notPlayingMode:
                command[2] = VCR_CMD_WIND
                if theSpeed == 0.0 {
                    command[3] = VCR_OPE_WIND_STOP
                } else if theSpeed > 0.0 {
                    command[3] = VCR_OPE_WIND_FAST_FORWARD
                } else {
                    command[3] = VCR_OPE_WIND_REWIND
                }
            @unknown default:
                return
            }

            var response: [UInt8] = [0, 0, 0, 0]
            var responseLen: UInt32 = 4
            objc_sync_enter(self)
            let result = iface.pointee.AVCCommand(avc, &command, UInt32(command.count),
                                                   &response, &responseLen)
            objc_sync_exit(self)
            if result != kIOReturnSuccess {
                NSLog("Error: Failed to send AVC command")
            }
            return
        }

        // Fallback to AVFoundation transport controls
        guard let device = device else { return }
        do {
            try device.lockForConfiguration()
            device.setTransportControlsPlaybackMode(theMode, speed: theSpeed)
            device.unlockForConfiguration()
        } catch {
            NSLog("Error: \(error)")
        }
    }

    // MARK: - AV/C Probing

    @objc public func probeAvcCommand(_ command: UInt8, operand: UInt8) -> Bool {
        guard let avc = avcDevice, let iface = avc.pointee else { return false }

        var inquiry: [UInt8] = [0x02, VCR_0, command, operand]
        var response: [UInt8] = [0, 0, 0, 0]
        var responseLen: UInt32 = 4

        objc_sync_enter(self)
        let result = iface.pointee.AVCCommand(avc, &inquiry, UInt32(inquiry.count),
                                               &response, &responseLen)
        objc_sync_exit(self)

        guard result == kIOReturnSuccess else { return false }
        return (response[0] & 0x0F) == 0x0C
    }

    @objc public func probeAvcGeneral(_ command: UInt8) -> Bool {
        guard let avc = avcDevice, let iface = avc.pointee else { return false }

        var inquiry: [UInt8] = [0x04, VCR_0, command, 0xFF]
        var response: [UInt8] = [0, 0, 0, 0]
        var responseLen: UInt32 = 4

        objc_sync_enter(self)
        let result = iface.pointee.AVCCommand(avc, &inquiry, UInt32(inquiry.count),
                                               &response, &responseLen)
        objc_sync_exit(self)

        guard result == kIOReturnSuccess else { return false }
        return (response[0] & 0x0F) == 0x0C
    }

    @objc public func queryAvcStatus(_ command: UInt8, operand: UInt8) -> UInt8 {
        guard let avc = avcDevice, let iface = avc.pointee else { return 0xFF }

        var statusCmd: [UInt8] = [0x01, VCR_0, command, operand]
        var response: [UInt8] = [0, 0, 0, 0]
        var responseLen: UInt32 = 4

        objc_sync_enter(self)
        let result = iface.pointee.AVCCommand(avc, &statusCmd, UInt32(statusCmd.count),
                                               &response, &responseLen)
        objc_sync_exit(self)

        guard result == kIOReturnSuccess else { return 0xFF }
        if (response[0] & 0x0F) == 0x0C {
            return response[3]
        }
        return 0xFF
    }

    // MARK: - Device Info from IOKit

    @objc public func getDeviceVendor() -> String {
        return lookupFireWireProperty("FireWire Vendor Name") ?? ""
    }

    @objc public func getDeviceModel() -> String {
        return lookupFireWireProperty("FireWire Product Name") ?? ""
    }

    private func lookupFireWireProperty(_ key: String) -> String? {
        guard let device = device else { return nil }

        var uniqueID = device.uniqueID
        if uniqueID.count > 2 && uniqueID.hasPrefix("0x") {
            uniqueID = String(uniqueID.dropFirst(2))
        }

        var iterator: io_iterator_t = 0
        guard IOServiceGetMatchingServices(kIOMainPortDefault,
                                            IOServiceNameMatching("IOFireWireDevice"),
                                            &iterator) == KERN_SUCCESS else { return nil }

        var service = IOIteratorNext(iterator)
        while service != 0 {
            var location = io_name_t()
            let locResult = withUnsafeMutablePointer(to: &location) { ptr in
                ptr.withMemoryRebound(to: CChar.self, capacity: MemoryLayout<io_name_t>.size) { charPtr in
                    IORegistryEntryGetLocationInPlane(service, kIOServicePlane, charPtr)
                }
            }
            let locationStr = withUnsafePointer(to: &location) { ptr in
                ptr.withMemoryRebound(to: CChar.self, capacity: MemoryLayout<io_name_t>.size) { charPtr in
                    String(cString: charPtr)
                }
            }

            if locResult == KERN_SUCCESS && locationStr == uniqueID {
                var properties: Unmanaged<CFMutableDictionary>?
                if IORegistryEntryCreateCFProperties(service, &properties, kCFAllocatorDefault, 0) == KERN_SUCCESS,
                   let dict = properties?.takeRetainedValue() as? [String: Any],
                   let value = dict[key] as? String {
                    IOObjectRelease(service)
                    IOObjectRelease(iterator)
                    return value
                }
                IOObjectRelease(service)
                break
            }
            IOObjectRelease(service)
            service = IOIteratorNext(iterator)
        }
        IOObjectRelease(iterator)
        return nil
    }

    // MARK: - Speed / Mode

    @objc public func getSpeed() -> Float {
        if let ctrl = externalController, ctrl.responds(to: #selector(getter: AVFCtl.oldSpeed)) {
            // External controller path - use perform
            return (ctrl.perform(Selector(("getSpeed")))?.takeUnretainedValue() as? NSNumber)?.floatValue ?? 0.0
        }
        objc_sync_enter(self)
        let speed = device?.transportControlsSpeed ?? 0.0
        objc_sync_exit(self)
        return speed
    }

    @objc public func getMode() -> AVCaptureDevice.TransportControlsPlaybackMode {
        if let ctrl = externalController, ctrl.responds(to: Selector(("getMode"))) {
            let result = (ctrl.perform(Selector(("getMode")))?.takeUnretainedValue() as? NSNumber)?.intValue ?? 0
            return AVCaptureDevice.TransportControlsPlaybackMode(rawValue: result) ?? .notPlayingMode
        }
        objc_sync_enter(self)
        let mode = device?.transportControlsPlaybackMode ?? .notPlayingMode
        objc_sync_exit(self)
        return mode
    }

    // MARK: - Wait

    @objc public func waitForSessionEnd(_ timeout: Int) -> Bool {
        // Wait for the device to start playing
        var counter = 0
        while getSpeed() == 0.0 && counter < 20 {
            Thread.sleep(forTimeInterval: 0.5)
            counter += 1
        }

        // Initialize receiver's last input timestamp
        receiverInstance?.setLastInput(Date())

        // Block as long as the capture session is running
        while getSpeed() != 0.0 {
            if timeout > 0, let lastInput = receiverInstance?.lastInput() {
                let deadline = lastInput.addingTimeInterval(TimeInterval(timeout))
                if deadline.compare(Date()) == .orderedAscending {
                    setPlaybackMode(.notPlayingMode, speed: 0.0)
                    return true
                }
            }
            Thread.sleep(forTimeInterval: 0.5)
        }

        return false
    }
}
