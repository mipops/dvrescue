---
layout: post
title: Troubleshooting
---

# Known Issues

Below is a list of known issues for the recent builds of DVRescue. Please see the linked issues on GitHub for more details.

- Windows/Linux connection with adapters doesn’t work (<a href="https://github.com/mipops/dvrescue/issues/718" target="_blank">Issue 1</a>, a href="https://github.com/mipops/dvrescue/issues/725" target="_blank">Issue 2</a>)
- <a href="https://github.com/mipops/dvrescue/issues/760" target="_blank">DVCPRO will not capture via dvrescue GUI, CLI or vrecord</a>
- <a href="https://github.com/mipops/dvrescue/issues/746" target="_blank">Deck control in the DVRescue GUI is buggy</a>


<details markdown="1">
  
<summary markdown="span"># No DV Deck Detected</summary>

<a href="{{ site.baseurl }}/images/no-deck-detected_small.gif"><img alt="No Deck Detected" src="{{ site.baseurl }}/images/no-deck-detected_small.gif"></a>

If your deck does not show up, try the following troubleshooting. There is also [a video guide](https://www.youtube.com/watch?t=232&v=7FaZw3RoVbA&feature=youtu.be) related to this troubleshooting.

After every step, re-launch DVRescue and check the capture tab to see if the device is detected. Restarting your computer and deck after major changes is also helpful.

## Connections

Start by ensuring that none of your cables are bent at a hard angle or straining to reach either the deck or the computer, and that all are properly supported. All three of these factors can cause cable failure. 

As noted in the <a href="{{ site.baseurl }}/sections/dv_transfer_station.html">Hardware Setup guide</a>, make sure that the port and cable you are using both have the Thunderbolt symbol. For some Mac laptops, the symbols are not included above the ports, but you can check the specifics by reviewing the [Apple Port Guide](https://support.apple.com/en-us/109523).

## Turn it On and Off Again!

The most time-honored of troubleshooting tactics.

Restart DVRescue and the deck.
- It can be helpful to do this a few times, changing the order of things slightly. For example, try turning the deck on before or after opening DVRescue.
- Always pause for a moment before turning the deck back on.
- Always wait at least a minute after closing DVRescue before opening it again.
- It can take DVRescue a few minutes to connect to the deck if you’re using a new daily build or release, or if it's the first time you’ve opened DVRescue.

If it is still not detected, restart your computer.
- Turn the deck off and close out of DVRescue
- Reboot the computer
- Once the computer reboots, turn the deck back on.
- Wait a few minutes and then open DVRescue again to check if it is connected.

If you’re still not connected to the deck, move onto further troubleshooting steps for your system.

<details markdown="1">

<summary markdown="span">## MacOS</summary>

### Check for the Device in the Command Line

Check whether the deck is detected through the command line by opening Terminal and running <code>dvrescue --list_devices</code>

This is more comprehensive and accurate than what is displayed in the GUI. If your deck is listed, then DVRescue is connected to it. Restart the DVRescue GUI and wait a few minutes to see if it shows up.

| <a href="{{ site.baseurl }}/images/dvrescue_no-devices.png"><img alt="DVRescue No Device" src="{{ site.baseurl }}/images/dvrescue_no-devices.png"></a> | <a href="{{ site.baseurl }}/images/dvrescue_one-devices.png"><img alt="DVRescue One Device" src="{{ site.baseurl }}/images/dvrescue_one-devices.png"></a> | <a href="{{ site.baseurl }}/images/dvrescue_two-devices.png"><img alt="DVRescue Two Devices" src="{{ site.baseurl }}/images/dvrescue_two-devices.png"></a> |

However, if your deck is not detected in the list of devices, next check whether it’s detected by other programs.

### Check for Device in Other Programs

This helps to know if the issue is with the connection between your computer and the deck or with only DVRescue.

#### vrecord

- In the Terminal, run <code>vrecord -e</code>
- Select the “DV” tab
- The device should appear in the list under the “Select a DV Device” section at the top of the window.
- If the device does not click the “Rescan” button (located below the list of the devices).
- If this doesn’t work, try waiting a few minutes and click “Rescan” again. It can sometimes take a few times before vrecord recognizes the device.

<a href="{{ site.baseurl }}/images/vrecord-check-02.png"><img alt="Vrecord Check" src="{{ site.baseurl }}/images/vrecord-check-02.png"></a>

#### QuickTime 

- Open Quicktime
- In the File menu, select “New Movie Recording”
- In the viewer that opens, click on the arrow next to the red record button.
- See if your device is listed in the menu that opens.

If vrecord and/or QuickTime can detect the deck, then you do have a connection. Return to DVRescue and again try restarting and waiting. Consider uninstalling and reinstalling DVRescue, using the most up-to-date version.

If the device does not show up in QuickTime or vrecord either, move on to check to see if the device is detected by your computer at all.

### Verify Firewire Connection

If you are certain that your settings are correct and you are still unable to see the device in any programs, check to see if the FireWire connection is working.

There are two ways you can access the list of connected devices.

#### Terminal

- Open the Terminal
- Run <code>avfctl -list_devices</code>
- This should generate a list of devices connected to your computer via FireWire.

<a href="{{ site.baseurl }}/images/terminal-avfctl.png"><img alt="Terminal avfctl" src="{{ site.baseurl }}/images/terminal-avfctl.png"></a>

Alternatively, you can run <code>ioreg -l</code> which will generate a list of all of the devices connected to your computer through various means. This list will be longer, but very comprehensive.

#### System Settings

- Go to About This Mac -> System Report
- Scroll down to Hardware/Thunderbolt
- The device(s) should be listed in the connected ports with the note “Device Connected”

<a href="{{ site.baseurl }}/images/firewire-devices.png"><img alt="Firewire Devices" src="{{ site.baseurl }}/images/firewire-devices.png"></a>

If the device **is not** detected in hardware, refer to the Failed Component section.

If the device **is** detected in hardware, but you are unable to establish a connection via vrecord, DVRescue or QuickTime, try the following steps to confirm your permissions and security policy are setup to be compatible with dvrescue.

### Verify Permissions, Security Policy, & System Integrity Protection

DVRescue requires some special permissions to access to operate and to capture DV videotape. These are all outlined in the <a href="{{ site.baseurl }}/sections/installation.html">Installation guide</a>. If any of these are not set correctly, they may block DVRescue from accessing connected devices to capture DV. Some of these settings may change or revert if your system has been updated.

#### Permissions

Both dvrescue and vrecord require permission to access your computer’s camera and screen recording in order to capture DV.

If for any reason the dvrescue installation does not prompt you to grant permission, you can fix the permissions manually for both programs:

- Ensure you are logged into your computer as an Administrator
- Open System Preferences
- Click on Security and Privacy
- Select the Privacy tab
- Fix Camera Permissions
  - Select Camera from the list
  - To make changes to the settings, you will need to unlock the lock located in the bottom left hand corner of the window. Click the lock and enter your password.
  - If not already included, add both of these items to the list. You can do so by clicking on the + button and selecting each from the applications list:
    - Terminal
    - dvrescue
  - If the Camera menu does not have the +/-...
    - Drag and drop DVRescue from the Applications folder into the permissions window.
    - You can add the Terminal and dvrescue to the Full Disk Access list instead.
    - Alternatively, you can disable the permissions settings automatically in place with most macOS software by following the steps below for disabling system integrity protection. (This is advised in order to prevent additional permissions related errors or limitations.) 
- Fix Screen Recording permissions
  - Select Screen Recording from the list on the left hand side of the Privacy window.
  - If not already included, add dvrescue to the list.
  - Make sure the checkbox is checked.

Finally, reboot your Mac for all changes to take effect.

#### Security Policy, & System Integrity Protection

PLEASE NOTE: after any automatic or manual updates to macOS (including security updates), check to see if the security policy was changed even if you have previously changed it.

Follow the steps outlined in the <a href="{{ site.baseurl }}/sections/installation.html">Installation guide</a> under “Recovery Mode Changes”.

In the majority of cases, following the preceding steps will fix the issue. But if, after following all of this, your system still will not detect your DV deck, move onto the below steps to determine if third party extensions are not being allowed.

### Third Party Extensions

#### Check for Device in BlackMagic Media Express

First check BlackMagic as well. Open BlackMagic Media Express. In the menu, choose Device. If it says “No device is connected” then this is further confirmation of the issue.

| <a href="{{ site.baseurl }}/images/blackmagic_device_connection_01.png"><img alt="Blackmagic No Device" src="{{ site.baseurl }}/images/blackmagic_device_connection_01.png"></a> | <a href="{{ site.baseurl }}/images/blackmagic_device_connection_02.png"><img alt="Blackmagic No Device" src="{{ site.baseurl }}/images/blackmagic_device_connection_02.png"></a> |

#### Reinstall BlackMagic Drivers

Even though you only need to use a BlackMagic video card for analog videotape captures, it is still required to run vrecord and dvrescue in general.

Before reinstalling BlackMagic drivers, double check the vrecord GitHub page, including the current issues, for conflicts with particular versions of BlackMagic. It is not always compatible with the current release of vrecord and can cause issues.

If there are no logged issues, then follow the next steps.

- Try a fresh install of BlackMagic Design drivers. If drivers are already installed, uninstall them using the uninstall command in the Blackmagic system folder.
- Download and install the drivers. The latest versions of BM software tend to be buggy, so just go with the oldest version of the drivers that your macOS can work with. 
- During the installation process, you should be prompted to allow Apple to use 3rd party extensions. Click “Open Security Preferences”
- System Preferences will open a new window with your Security Preferences. Click on the General tab.
- If it’s not already unlocked, click on the lock icon in the lower left hand corner of the window. Enter your password when prompted. 
- Click “Allow” near the bottom of the window.
- Make sure to relock the lock to save the changes you have made to your Security Preferences.
- Shut down your computer (don’t restart if prompted). 

| <a href="{{ site.baseurl }}/images/blackmagic_drivers_install_01.jpg"><img alt="Blackmagic Drivers Installation" src="{{ site.baseurl }}/images/blackmagic_drivers_install_01.jpg"></a> | <a href="{{ site.baseurl }}/images/blackmagic_drivers_install_02.jpg"><img alt="Blackmagic Drivers Installation" src="{{ site.baseurl }}/images/blackmagic_drivers_install_02.jpg"></a> | 

</details>

<details markdown="1">

<summary markdown="span">## Windows/Linux/Ubuntu</summary>

At this time, DVRescue offers limited support for non-OSX systems, although we hope to expand in the future. If you are using Windows, Linux, or Ubuntu and DVRescue still isn’t able to interact with your DV deck, or if you have any other trouble with the software, you may need to do some research in order to troubleshoot.

Some versions of Ubuntu might need permissions to be edited to allow DVRescue access. See <a href="https://github.com/mipops/dvrescue/issues/514" target="_blank">this issue</a> on the project page for a possible solution.

If you suspect a hardware rather than a software issue, review the Failed Component troubleshooting section for general advice. Also peruse the <a href="https://github.com/mipops/dvrescue/issues" target="_blank">GitHub Issues page</a> for any discussions that might relate to your issue.

If the deck is not detected in your computer’s FireWire connections even after you have checked your cables and restarted your computer and deck, as outlined above, then it is likely that one of the physical components of your connection has failed.

</details>

<details markdown="1">

## Failed Component

Try using a different deck, cable, or adapter, making sure to change only one variable at a time in order to hone in on the part that is not working. The most common issue would be a failed cord or adapter. Unfortunately, Apple adapters seem to have a longer lifespan than other brands at this time. For a step-by-step guide to ensure you determine the correct variable, see the following section.

For most of these tips, the assumption is that you have additional cables, adapters and FireWire devices. It is our recommendation that when you purchase a cable or adapter that you buy at least two (or more) at a time to ensure that you have a replacement for testing and in the event of cable failure.

<a href="{{ site.baseurl }}/images/transfer-setup_troubleshooting-chart.png">"<img alt="Hardware Troubleshooting" src="{{ site.baseurl }}/images/transfer-setup_troubleshooting-chart.png"></a>

### Try another device

- Try connecting your FireWire cable to either a different deck or another FireWire compatible device (such as a legacy hard drive or camera) and see if the device shows up (using the steps “Establishing Deck to Computer Connection” and “Restarting” sections above). 
- If the second device shows up, there may be an issue with the first deck you were attempting to use (likely the FireWire output is broken). 
- Try connecting the first device again. If it shows up, you are good to go. If it fails to connect, we would recommend having the device inspected for possible damage. 
- If the second device does not show up, move onto the next section.

### Try another cable

- Preferably using a brand new cable, re-connect your deck to the computer (either with or without adapters depending on your set-up).
- Repeat the steps listed in the “Establishing Deck to Computer Connection” section as well as the above troubleshooting steps, if needed.
- If the deck still does not show up, move on to the next section.

### Try another adapter:

- If you are using more than one adapter, follow these steps for each one, switching out one at a time. 
- Preferably, using a brand new adapter, try connecting your deck to the computer.
- Repeat the steps listed in the “Establishing Deck to Computer Connection” section as well as the above troubleshooting steps, if needed.
- If the deck still does not show up and you are using more than one adapter, try switching out the second one.

If none of these components can be pinpointed as the source of the issue, consider whether there might be a problem with your computer’s FireWire port. If you can, attach a different FireWire device. Alternatively, test the same setup using another computer.

## Last resort
If none of this works, your problem may be more complicated. You can always visit the <a href="https://github.com/mipops/dvrescue" target="_blank">DVRescue GitHub page</a> as a resource to see if anyone else is having similar problems or to ask questions and report issues. If you do want to report an issue, provide as much information as possible, including the devices and operating system you are using!

</details>

</details>

<details markdown="1">
  
# dvcapture via vrecord

If you are using dvcapture through vrecord, in general please refer to the vrecord documentation on GitHub for troubleshooting. However, there are a few known issues:

## Bitstream Error Concealment

<a href="{{ site.baseurl }}/images/vrecord_bitstream_errors.png">"<img alt="vrecord bitstream errors" src="{{ site.baseurl }}/images/vrecord_bitstream_errors.png"></a>

If vrecord detects that the DV device is concealing bitstream errors, this will be noted in the Terminal. Error concealment is a built-in function of DV tapes/deck communication.

## vrecord Permissions Error

If you receive the following error while capturing via vrecord using macOS 10.14.6:

<a href="{{ site.baseurl }}/images/vrecord_permissions_error.png"><img alt="vrecord permissions errors" src="{{ site.baseurl }}/images/vrecord_permissions_error.png"></a>

The key part here is "this app is not authorized to use DV-VCR" (the name of the deck noted in the error will reflect the one you selected from the list in the vrecord “Edit Settings” menu).

You will need to change the OS camera permissions by following these steps:

- Ensure you are logged into your computer as the Administrator
- Open System Preferences
- Click on Security and Privacy
- Select the Privacy tab
- Select Camera from the list on the left hand side of the window
- To make changes to the settings, you will need to unlock the lock located in the bottom left hand corner of the window, by clicking on the lock and entering your password when prompted.
- If not already included, add the Terminal to the list, by clicking on the + button and selecting it from from the applications list.
- If the Camera menu does not have the +/- you can add the Terminal to the Full Disk Access list instead. 
- Reboot your Mac for the changes to take effect.

</details>



