/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

import AppKit
import Foundation

// NSOpenSavePanelDelegate implementation for directory selection
private class PanelDelegate: NSObject, NSOpenSavePanelDelegate {
    let dirURL: URL

    init(directoryURL: URL) {
        dirURL = directoryURL
        super.init()
    }
}

/// Query read/write permission for a directory path via macOS sandbox dialog.
/// This is called from C++ via the Objective-C bridging header.
/// - Parameters:
///   - dirPath: The directory path to request access to
///   - message: The message to display in the open panel
/// - Returns: The selected directory path, or an empty string if cancelled
@objc public func sandboxQueryRWPermissionForPathSwift(_ dirPath: NSString, message: NSString) -> NSString {
    let dirURL = URL(fileURLWithPath: dirPath as String)
    let delegate = PanelDelegate(directoryURL: dirURL)

    let openPanel = NSOpenPanel()
    openPanel.canChooseFiles = false
    openPanel.canCreateDirectories = false
    openPanel.canChooseDirectories = true
    openPanel.allowsMultipleSelection = false
    openPanel.directoryURL = dirURL
    openPanel.message = message as String
    openPanel.delegate = delegate

    if openPanel.runModal() == .OK, let url = openPanel.url {
        return url.path as NSString
    }

    return "" as NSString
}
