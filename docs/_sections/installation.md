---
layout: post
title: Installation
---

## Installation Guide
Releases of DVRescue can be downloaded for all supported systems from its page on the [MediaArea website](https://mediaarea.net/DVRescue).

It also is possible to download specific snapshots and daily builds from MediaArea via their download page [here](https://old.mediaarea.net/download/snapshots/binary/dvrescue/).

It is also possible to use a package managers to install DVRescue. If using [Homebrew](https://brew.sh/) tap the MediaArea repository with the command `brew tap mediaarea/homebrew-mediaarea` and then install with `brew install dvrescue`

For other package managers, see the instructions provided on the [MediaArea Software Repositories](https://mediaarea.net/en/Repos ) page.

## Permissions (Mac Specific)
Dvrescue requires permission to access your computer’s camera. The dvrescue installation should prompt you to do this, but to ensure that both programs will work properly, you can check the permissions by using the following steps:

* Ensure you are logged into your computer as the Administrator
* Open System Preferences
* Click on Security and Privacy
* Select the Privacy tab
* Select Camera from the list on the left hand side of the window
* To make changes to the settings, you will need to unlock the lock located in the bottom left hand corner of the window, by clicking on the lock and entering your password when prompted.
* If not already included, add both of these items to the list, by clicking on the + button and selecting each from the applications list:
  - Terminal
  - dvrescue
 * If the Camera menu does not have the +/- you can add the Terminal and dvrescue to the Full Disk Access list instead. Alternatively, you can disable the permissions settings automatically in place with most macOS software by following the steps below (this is advised in order to prevent additional permissions related errors or limitations).
 * Additionally, dvrescue will require permission to record your screen. This should also be part of the installation prompts, but to ensure the permission is included:
   - Select Screen Recording from the list on the left hand side of the Privacy window.
   - If not already included, add dvrescue to the list.
   - Make sure the checkbox is checked.
 * Reboot your Mac for the changes to take effect.

 _Disable System Integrity Protection_

 This step may also be necessary on a Mac. To disable SIP, do the following:
 * Restart your computer in [Recovery mode](https://support.apple.com/en-us/HT201314). Turn on your Mac and immediately press and hold these two keys: Command (⌘) and R. Release the keys when you see an Apple logo, spinning globe, or other startup screen.
 * Launch Terminal from the Utilities menu.
 * Run the command csrutil disable
 * Restart your computer.