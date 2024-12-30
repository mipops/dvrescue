---
layout: post
title: Troubleshooting
---

If you run into bugs or other trouble while using DVRescue, check whether your issue matches any of the common issues below and follow the troubleshooting steps.

When troubleshooting any issue, ensure that you are using the most current version of DVRescue, <a href="https://mediaarea.net/DVRescue" target="blank">which you can download here</a>. 

## Don't see your issue anywhere?

Read below about how to use GitHub to search and report new issues.

<details markdown="1">

<summary markdown="span">Guide</summary>

The GitHub issues page is where you can report, discuss, and resolve problems you are experiencing with DVRescue.

Search for a few keywords to see whether your issue, or something similar, has been reported already. You can contribute to that existing discussion if desired, or follow any steps currently being developed to fix the problem.

To report a new issue, first do the following:
- Make sure you are using the most current version of DVRescue. Open the program and select "about DVRescue" in the top left menu. Make a note of the version date.
- Also make a note of your machine and OS version under "About this Mac"
- Open the Terminal.app and launch DVRescue by running this command: `/Applications/dvrescue.app/Contents/MacOS/dvrescue --log`
- Recreate the issue using the fewest number of steps possible
- Quit DVRescue
- Go back to Terminal and run this command: `cat ~/Library/Containers/net.MediaArea.dvrescue.mac-gui/Data/dvrescue-gui_logs/dvrescue-gui.log`
- Either copy the contents or go to `Shell -> Export Text As` to save as a text document
- In GitHub, you will need to be logged into your account. Select the "New issue" button and give your issue a succinct and specific title
- Describe your issue as clearly as you can. Attach or copy-paste the log you just saved. Make sure to specify your computer, its OS, and the version of DVRescue

You should be automatically subscribed to receive notifications when others reply or add to your issue. Keep an eye on it because the developers may ask follow up questions, ask you to try to replicate your issue with different steps, or ask you whether new DVRescue updates have fixed the issue.

</details>

## Known Issues

Below is a list of known issues for the recent builds of DVRescue. Please see the linked issues on GitHub for more details.

- Windows/Linux connection with adapters doesn’t work (<a href="https://github.com/mipops/dvrescue/issues/718" target="_blank">Issue 1</a>, <a href="https://github.com/mipops/dvrescue/issues/725" target="_blank">Issue 2</a>)
- <a href="https://github.com/mipops/dvrescue/issues/746" target="_blank">Deck control in the DVRescue GUI is buggy</a>
- <a href="https://github.com/mipops/dvrescue/issues/760" target="_blank">DVCPRO will not capture via dvrescue GUI, CLI or vrecord</a>
- <a href="https://github.com/mipops/dvrescue/issues/930" target="_blank">DVRescue crashing when merging files</a>
- <a href="https://github.com/mipops/dvrescue/issues/950" target="_blank">DVpackager Rewrapped MOV Files Broken Header</a>
&nbsp;

# No DV Deck Detected

<details markdown="1">

<summary markdown="span">Guide</summary>

<a href="{{ site.baseurl }}/images/no-deck-detected_small.gif"><img alt="No Deck Detected" src="{{ site.baseurl }}/images/no-deck-detected_small.gif"></a>

If your deck does not show up, try the following troubleshooting. There is also [a video guide](https://www.youtube.com/watch?t=232&v=7FaZw3RoVbA&feature=youtu.be) related to this troubleshooting.

After every step, re-launch DVRescue and check the capture tab to see if the device is detected. Restarting your computer and deck after major changes is also helpful.

## Check Connections

<details markdown="1">

Start by ensuring that none of your cables are bent at a hard angle or straining to reach either the deck or the computer, and that all are properly supported. All three of these factors can cause cable failure. 

As noted in the <a href="{{ site.baseurl }}/sections/dv_transfer_station.html">Hardware Setup guide</a>, make sure that the port and cable you are using both have the Thunderbolt symbol. For some Mac laptops, the symbols are not included above the ports, but you can check the specifics by reviewing the [Apple Port Guide](https://support.apple.com/en-us/109523).

</details>
&nbsp;

## Turn it On and Off Again!

<details markdown="1">

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

</details>
&nbsp;

## macOS Steps

<details markdown="1">
&nbsp;

### Check for the Device in the Command Line

<details markdown="1">

Check whether the deck is detected through the command line by opening Terminal and running `dvrescue --list_devices`

This is more comprehensive and accurate than what is displayed in the GUI. If your deck is listed, then DVRescue is connected to it. Restart the DVRescue GUI and wait a few minutes to see if it shows up.

| <a href="{{ site.baseurl }}/images/dvrescue_no-devices.png"><img alt="DVRescue No Device" src="{{ site.baseurl }}/images/dvrescue_no-devices.png"></a> | <a href="{{ site.baseurl }}/images/dvrescue_one-devices.png"><img alt="DVRescue One Device" src="{{ site.baseurl }}/images/dvrescue_one-devices.png"></a> | <a href="{{ site.baseurl }}/images/dvrescue_two-devices.png"><img alt="DVRescue Two Devices" src="{{ site.baseurl }}/images/dvrescue_two-devices.png"></a> |

If your deck is not detected in the list of devices, next check whether it’s detected by other programs.

</details>
&nbsp;

### Check for Device in Other Programs

This helps to know if the issue is with the connection between your computer and the deck or with only DVRescue.

<details markdown="1">

#### vrecord

- In the Terminal, run `vrecord -e`
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

</details>
&nbsp;

### Verify Firewire Connection

<details markdown="1">

There are two ways you can access the list of connected devices to see if the FireWire connection is working at all.

#### Terminal

- Open the Terminal
- Run `avfctl -list_devices`
- This should generate a list of devices connected to your computer via FireWire.

<a href="{{ site.baseurl }}/images/terminal-avfctl.png"><img alt="Terminal avfctl" src="{{ site.baseurl }}/images/terminal-avfctl.png"></a>

Alternatively, you can run `ioreg -l` which will generate a list of all of the devices connected to your computer through various means. This list will be longer, but very comprehensive.

#### System Settings

- Go to About This Mac -> System Report
- Scroll down to Hardware/Thunderbolt
- The device(s) should be listed in the connected ports with the note “Device Connected”

<a href="{{ site.baseurl }}/images/firewire-devices.png"><img alt="Firewire Devices" src="{{ site.baseurl }}/images/firewire-devices.png"></a>

If the device **is not** detected in hardware, refer to the Failed Component section.

If the device **is** detected in hardware, but you are unable to establish a connection via vrecord, DVRescue or QuickTime, try the following steps to confirm your permissions and security policy are setup to be compatible with dvrescue.

</details>
&nbsp;

### Check for Issues with BlackMagic

If all of the previous steps have failed, there may be an issue with your Blackmagic drivers. Even though you only directly use a BlackMagic video card for analog videotape captures, it is still required to run vrecord and dvrescue in general.

<details markdown="1">

#### Check for Device in BlackMagic Media Express

First check BlackMagic as well. Open BlackMagic Media Express. In the menu, choose Device. If it says “No device is connected” then this is further confirmation of the issue.

<p float="left">
<a href="{{ site.baseurl }}/images/blackmagic_device_connection_01.png"><img alt="Blackmagic No Device" src="{{ site.baseurl }}/images/blackmagic_device_connection_01.png"></a>
<a href="{{ site.baseurl }}/images/blackmagic_device_connection_02.png"><img alt="Blackmagic No Device" src="{{ site.baseurl }}/images/blackmagic_device_connection_02.png"></a>
</p>

<br />

#### Reinstall BlackMagic Drivers

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

<p float="left">
<a href="{{ site.baseurl }}/images/blackmagic_drivers_install_01.jpg"><img alt="Blackmagic Drivers Installation" src="{{ site.baseurl }}/images/blackmagic_drivers_install_01.jpg"></a>
<a href="{{ site.baseurl }}/images/blackmagic_drivers_install_02.png"><img alt="Blackmagic Drivers Installation" src="{{ site.baseurl }}/images/blackmagic_drivers_install_02.png"></a>
</p>

</details>
&nbsp;

</details>
&nbsp;

## Windows/Linux/Ubuntu Steps

<details markdown="1">

Note that capture is not supported at this time in the GUI on windows/Linux/Ubuntu. In order to capture on Windows, you will need a dual boot of Windows/Ubuntu and you will need to use the CLI or dvcapture via vrecord.

Check for your device in the command line with `dvrescue --list_devices` and in vrecord with `vrecord -e`, selecting the “DV” tab. If the device is detected there, you should be able to capture.

Some versions of Ubuntu might need permissions to be edited to allow DVRescue access. See <a href="https://github.com/mipops/dvrescue/issues/514" target="_blank">this issue</a> on the project page for a possible solution.

At this time, DVRescue offers limited support for non-OSX systems, although we hope to expand in the future. If you are using Windows, Linux, or Ubuntu and DVRescue still isn’t able to interact with your DV deck, or if you have any other trouble with the software, you may need to do some research in order to troubleshoot. You can start by perusing the <a href="https://github.com/mipops/dvrescue/issues" target="_blank">GitHub Issues page</a> for any discussions that might relate to your issue. Search "windows," "ubuntu" or other keywords as appropriate to locate relevate issues.

</details>
&nbsp;

## Failed Component

If the deck is not detected in your computer’s FireWire connections even after you have checked your cables, restarted your computer and deck, and followed all other troubleshooting above, then it is likely that one of the physical components of your connection has failed.

<details markdown="1">

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

</details>
&nbsp;

## Last resort
If none of this works, your problem may be more complicated. You can always visit the <a href="https://github.com/mipops/dvrescue" target="_blank">DVRescue GitHub page</a> as a resource to see if anyone else is having similar problems or to ask questions and report issues. If you do want to report an issue, provide as much information as possible, including the devices and operating system you are using!

</details>
&nbsp;

# Permissions, Security Policy, & System Integrity Protection (macOS)

DVRescue requires some special permissions to access to operate and to capture DV videotape. (These are all also outlined in the <a href="{{ site.baseurl }}/sections/installation.html" target="_blank">Installation guide</a>.) If any of these are not set correctly, they may block DVRescue from accessing connected devices to capture DV.

<div class="blockquote">
PLEASE NOTE: after any automatic or manual updates to macOS (including security updates), check to see if the security policy and permissions have changed, even if you have previously set them.
</div>
<br/>

<details markdown="1">

<summary markdown="span">Guide</summary>

## Recovery Mode Changes

DVRescue requires some special permissions to operate and to capture DV videotape.<br>*Example images taken from a M1 Mac Mini 2020+ running Monterey 12.4+*

<details markdown="1">

### Disable System Integrity Protection

<details markdown="1">

To disable SIP, do the following:
- Restart your computer in <a href="https://support.apple.com/en-us/102518" target="_blank">Recovery mode</a>. Methods will vary based on Mac model:
  - <a href="https://support.apple.com/guide/mac-help/use-macos-recovery-on-an-intel-based-mac-mchl338cf9a8/mac" target="_blank">Intel-based Mac computers</a>
  - <a href="https://support.apple.com/guide/mac-help/macos-recovery-a-mac-apple-silicon-mchl82829c17/mac" target="_blank">Mac with Apple Silicon</a>
- Launch Terminal from the Utilities menu.
- Run the command `csrutil disable`
- Enter password if requested
- Restart your computer

</details>
&nbsp;

### Change the security policy

<details markdown="1">

- Restart your computer in <a href="https://support.apple.com/en-us/102518" target="_blank">Recovery mode</a>. Methods will vary based on Mac model:
  - <a href="https://support.apple.com/guide/mac-help/use-macos-recovery-on-an-intel-based-mac-mchl338cf9a8/mac" target="_blank">Intel-based Mac computers</a>
  - <a href="https://support.apple.com/guide/mac-help/macos-recovery-a-mac-apple-silicon-mchl82829c17/mac" target="_blank">Mac with Apple Silicon</a>

- In the Recovery app, choose Utilities > Startup Security Utility.
- Select the system you want to use to set the security policy.
- If the disk is encrypted with FileVault, click Unlock, enter the password, then click Unlock.
- Click Security Policy
- Depending on the model of your computer, the Security Policy might be slightly different. Select your model:

<details markdown="1">

<summary markdown="span">M1 Mac</summary>

- Select **Reduced Security**: Allows any version of signed operating system software ever trusted by Apple to run.
- Under Reduced Security, there are two additional options. Ensure the box next to the first option is checked. If needed for remote access, check the second box as well:
  - **Allow user management of kernel extensions from identified developers**: Allow installation of software that uses legacy kernel extensions.
  - **Allow remote management of kernel extensions and automatic software updates**: Authorize remote management of legacy kernel extensions and software updates using a mobile device management (MDM) solution.
- Click OK.
- *If applicable for multiple users/your computer settings*: If you changed the security policy, click the User pop-up menu, choose an administrator account, enter the password for the administrator account, then click OK.

</details>

<details markdown="1">

<summary markdown="span">2019 Macbook Pro</summary>

- Select **Medium Security**: Allows any version of signed operating system software ever trusted by Apple to run.
- Under Medium Security, there are two additional options. Ensure the box next to the first option is checked. If needed for remote access, check the second box as well:
  - **Allow user management of kernel extensions from identified developers**: Allow installation of software that uses legacy kernel extensions.
  - **Allow remote management of kernel extensions and automatic software updates**: Authorize remote management of legacy kernel extensions and software updates using a mobile device management (MDM) solution.
- Click OK.
- *Only if the computer has multiple users and you are not logged in as an admin*: If you changed the security policy, click the User pop-up menu, choose an administrator account, enter the password for the administrator account, then click OK.

</details>

<details markdown="1">

<summary markdown="span">2017 Mac Pro</summary>

- Select **Reduced Security**: Allows any version of signed operating system software ever trusted by Apple to run.
- Under Reduced Security, there are two additional options. Ensure the box next to the first option is checked. If needed for remote access, check the second box as well:
  - **Allow user management of kernel extensions from identified developers**: Allow installation of software that uses legacy kernel extensions.
  - **Allow remote management of kernel extensions and automatic software updates**: Authorize remote management of legacy kernel extensions and software updates using a mobile device management (MDM) solution.
- Click OK.
- *Only if the computer has multiple users and you are not logged in as an admin*: If you changed the security policy, click the User pop-up menu, choose an administrator account, enter the password for the administrator account, then click OK.

</details>

- For all models, you must restart your Mac for the changes to take effect. Choose Apple menu  > Restart. Allow the computer to restart as normal.
- Once the computer is fully restarted, go to System Preferences again: System Preferences/Security Preferences
- Click on the General tab. If the system software message is still present, complete the following steps.
  - If it’s not already unlocked, click on the lock icon in the lower left hand corner of the window. Enter your password when prompted.
  - Click “Allow” near the bottom of the window.
  - Make sure to relock the lock to save the changes you have made to your Security Preferences.
- Restart your computer again.

Once the computer has restarted again, connect a DV device and turn it on. Open DVRescue and navigate to the Capture tab. See if the program detects your device and continue to follow the steps below.

</details>
&nbsp;

</details>
&nbsp;

## Pop-up permissions

<details markdown="1">

### Rosetta (M1 only)

The first time you open the DVRescue GUI, a pop-up window may open that prompts you to install Rosetta.
(This depends on your computer’s hardware and whether you have already downloaded Rosetta for another program.)
Click “install” in the pop-up window.

### Camera and screen recording access

- Click on the “Capture” tab.
- Plug in the deck you wish to use.
- Turn on the deck.
- When prompted, grant DVRescue permission to access your camera.
- You should get a similar prompt asking for permission for dvrescue to record your screen the first time you capture DV.

After granting these permissions, you will then be able to use all of the DVRescue tools in the GUI.

</details>
&nbsp;

## System Preferences permissions

Both dvrescue and vrecord require permission to access your computer’s camera and screen recording in order to capture DV. If for any reason the dvrescue installation does not prompt you to grant permission, you can fix the permissions manually for both programs:

<details markdown="1">

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

</details>
&nbsp;

</details>
&nbsp;

# Firewire permissions (Ubuntu)

<details markdown="1">

<summary markdown="span">Guide</summary>

<a href="https://github.com/mipops/dvrescue/issues/514" target="_blank">Github Issue: Firewire permissions issue in Ubuntu 22.04</a>

After upgrading to 22.04, there may be a change in levels of permission that disables Firewire devices for a user. This blocks dvrescue components such as dvcont and FFmpeg from communicating with the deck.

Solution was found in <a href="https://superuser.com/questions/236562/permanently-changing-permission-of-firewire-device" target="_blank"> another forum</a>:

Make a `/etc/udev/rules.d` directory:

`sudo mkdir /etc/udev/rules.d`

Next define a new rule:

`sudo -s
echo 'KERNEL=="fw*", GROUP="video", MODE="0664"' > /etc/udev/rules.d/80-firewire.rules
exit`

Then user needs to be added to the video group with `usermod -a -G video USERNAME`

</details>

# dvcapture via vrecord

<details markdown="1">

<summary markdown="span">Guide</summary>

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
&nbsp;

