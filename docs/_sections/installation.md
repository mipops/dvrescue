---
layout: post
title: Installation
---

# Mac Installation Guide

<details markdown="1">

## Installing DVRescue

<details markdown="1">

<summary markdown="span">Using Homebrew</summary>

#### Installing Homebrew

If you do not already have Homebrew installed, follow the steps below to install it. If you do, skip to the next section for updates/upgrades. 

- Establish an internet connection.
- Install Homebrew, by clicking on <a href="https://brew.sh/" target="_blank">this link</a> and copy the script provided.
- Paste the script into your macOS Terminal or Linux shell prompt.
- The script explains what it will do and then pauses before it does it. This process takes awhile to complete (roughly 20-40 minutes depending on your computer and internet speed).
- Check for any warnings or follow-up actions provided in the Terminal.
- To ensure everything is up to date, run the following commands in the Terminal (copy and paste them in one at a time, hit Return after each, and allow each to complete prior to running the next command):
  - `brew update`
  - `brew upgrade`
  - `brew cleanup`
  - `brew doctor`

#### Installing DVRescue

In the Terminal, run the following commands to install the command line tools for dvrescue:

  `brew tap mediaarea/homebrew-mediaarea`

  `brew install dvrescue`

Run all of the brew update commands again to make sure that everything is up-to-date and ready to go:

- `brew update`
- `brew upgrade`
- `brew cleanup`
- `brew doctor`

</details>

<details markdown="1">

<summary markdown="span">Direct Download</summary>

You can also install either the GUI version of the program or the Command Line Interface (CLI) <a href="https://mediaarea.net/DVRescue" target="_blank">directly from MediaArea.</a>

**Graphic User Interface (GUI)**

Please note that if you download the GUI, you do not need to download the CLI separately.

- Visit the <a href="https://mediaarea.net/DVRescue" target="_blank">MediaArea DVRescue page</a>
- Below the “Download” button, click “GUI” next to “very latest snapshots”
  - You can also access the most recent snapshots by going to the <a href="https://mediaarea.net/download/snapshots/binary/dvrescue/" target="_blank">MediaArea daily builds page</a>, clicking on “Parent Directory,” scrolling down to “dvrescue-gui,” and clicking on the folder.
- Click on the topmost folder (underneath the “ParentDirectory” folder), which should have the most recent date.
- Scroll down to the bottom of the page and select the .dmg file.
- Follow prompts for selecting the download location
- Navigate to where you saved the file
- Double-click on the file to open it
- In the window that pops up, drag the DVRescue application icon into the “Applications” folder.
  - If adding a new version, click “replace” when prompted.

**Command Line Tools (CLI)**

- Visit the <a href="https://mediaarea.net/DVRescue" target="_blank">MediaArea DVRescue page</a>
- Below the “Download” button, click “CLI” next to “very latest snapshots”
  - You can also access the most recent snapshots by going to the <a href="https://mediaarea.net/download/snapshots/binary/dvrescue/" target="_blank">MediaArea daily builds page</a>, clicking on “Parent Directory,” scrolling down to “dvrescue,” and clicking on the folder.
- Scroll down to the bottom of the page and select the **.dmg** file.
- Follow prompts for selecting the download location
- Navigate to where you saved the file
- Double-click on the file to open it
- Follow prompts for installation.

</details>

## Setting Up

DVRescue requires some special permissions to operate and to capture DV videotape.<br>*Example images taken from a M1 Mac Mini 2020+ running Monterey 12.4+*

### Recovery Mode Changes
Please note: in order for your computer to allow DVRescue to interact with your camera and microphone (which allows it to record and detect the connection via FireWire), you **need** to change the following settings for your security policy and permissions.

<details markdown="1"> 

<summary markdown="span">Disable System Integrity Protection</summary>

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

<details markdown="1">

<summary markdown="span">Change the security policy</summary>

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

### Rosetta (M1 only)

<a href="{{ site.baseurl }}/images/Rosetta.png"><img alt="rosetta" src="{{ site.baseurl }}/images/Rosetta.png" style="float: right; margin-left: 15px; width: 300px"></a>

- The first time you open the DVRescue GUI, a pop-up window may open that prompts you to install Rosetta.
  - (This depends on your computer’s hardware and whether you have already downloaded Rosetta for another program.)
- Click “install” in the pop-up window.

### Permissions

<a href="{{ site.baseurl }}/images/Camera-Permission.png"><img alt="camera permissions" src="{{ site.baseurl }}/images/Camera-Permission.png" style="float: right; margin-left: 15px; width: 300px"></a>

Both dvrescue and vrecord require permission to access your computer’s camera and screen recording in order to capture DV.

- Click on the “Capture” tab.
- Plug in the deck you wish to use.
- Turn on the deck.
- When prompted, grant DVRescue permission to access your camera.
- You should get a similar prompt asking for permission for dvrescue to record your screen the first time you capture DV.

After granting these permissions, you will then be able to use all of the DVRescue tools in the GUI.

If for any reason the dvrescue installation does not prompt you to grant permission for either the camera or screen recording, you can fix the permissions manually for both programs through System Preferences. Refer to the <a href="{{ site.baseurl }}/sections/troubleshooting.html" target="_blank">Troubleshooting documentation</a> for detailed instructions.

</details>

&nbsp;

# Linux/Windows Installation Guide

<details markdown="1">

## Installing DVRescue

You can install either the GUI version of the program or the Command Line Interface (CLI) <a href="https://mediaarea.net/DVRescue" target="_blank">directly from MediaArea.</a>

**Graphical User Interface (GUI)**

- Visit the <a href="https://mediaarea.net/DVRescue" target="_blank">MediaArea DVRescue page</a>
- Below the “Download” button, click “GUI” next to “very latest snapshots”
  - You can also access the most recent snapshots by going to the <a href="https://mediaarea.net/download/snapshots/binary/dvrescue/" target="_blank">MediaArea daily builds page</a>, clicking on “Parent Directory,” scrolling down to “dvrescue-gui,” and clicking on the folder.
- Click on the topmost folder (underneath the “ParentDirectory” folder), which should have the most recent date. Look for a file appropriate to your system.
  - For Windows: This will be a file ending in Windows.exe or Windows_x64.zip
- If you do not see a file of the necessary type, go back to the last folder and try the next-most recent date. Continue this until you find a folder with a file of the correct type.
- Follow prompts for selecting the download location
- Navigate to where you saved the file
- Double-click on the file to open it
- Follow prompts for installation

**Command Line Tools (CLI)**

- Visit the <a href="https://mediaarea.net/DVRescue" target="_blank">MediaArea DVRescue page</a>
- Below the “Download” button, click “CLI” next to “very latest snapshots”
  - You can also access the most recent snapshots by going to the <a href="https://mediaarea.net/download/snapshots/binary/dvrescue/" target="_blank">MediaArea daily builds page</a>, clicking on “Parent Directory,” scrolling down to “dvrescue,” and clicking on the folder.
- Click on the topmost folder (underneath the “ParentDirectory” folder), which should have the most recent date. Look for a file appropriate to your system.
  - For Windows: This will be a file ending in Windows.exe or Windows_x64.zip
- If you do not see a file of the necessary type, go back to the last folder and try the next-most recent date. Continue this until you find a folder with a file of the correct type.
- Follow prompts for selecting the download location
- Navigate to where you saved the file
- Double-click on the file to open it
- Follow prompts for installation.

It also is possible to download the most up to date development versions of the tool via the MediaArea daily builds pages. These are available at the following locations:

- CLI daily builds: <a href="https://mediaarea.net/download/snapshots/binary/dvrescue/" target="_blank">https://mediaarea.net/download/snapshots/binary/dvrescue/</a>
- GUI daily builds: <a href="https://mediaarea.net/download/snapshots/binary/dvrescue-gui/" target="_blank">https://mediaarea.net/download/snapshots/binary/dvrescue-gui/</a>

Linux users also have the option of activating the MediaArea repositories via the instructions on <a href="https://mediaarea.net/en/Repos" target="_blank">this page</a>. This enables installing and updating dvrescue and dvrescue-gui through the system package manager.

## Dependencies

For DVRescue to function it needs FFmpeg, mediainfo, xmlstarlet to be installed and present. These are bundled in the GUI installer, so if you are running the GUI you shouldn’t need to worry about this. For the CLI install you may need to install missing dependencies yourself if they are not present on your system.

Also - some Linux systems may have older versions of FFmpeg installed that do not support full DVRescue functionality. It is recommended on Linux to install both the GUI and the CLI for this reason, even if you are just intending to use the CLI as the CLI will be able to use the FFmpeg version packaged inside the GUI.

</details>

&nbsp;
