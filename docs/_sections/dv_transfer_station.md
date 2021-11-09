---
layout: post
title: DV Transfer Station
---


## Installation
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

 To disable SIP, do the following:
 * Restart your computer in [Recovery mode](https://support.apple.com/en-us/HT201314). Turn on your Mac and immediately press and hold these two keys: Command (⌘) and R. Release the keys when you see an Apple logo, spinning globe, or other startup screen.
 * Launch Terminal from the Utilities menu.
 * Run the command csrutil disable
 * Restart your computer.

## Equipment Set-up

1) Using the Cable Guide, determine the types of cables, adapters and power supply needed to connect your specific deck to the generation of Mac you are using. Ensure that any Thunderbolt cables are compatible with data transfer (vs. for display only).

2) Plug your deck into its power supply and into a power outlet. Make sure that the cables are inserted all the way into the output port on the back of the deck.

3) Connect your deck to the computer using the required cable (and adapters if needed). For details, see the images below for various ways to set-up the connection based on the input ports available on your computer and the DV IN/OUT on the deck you are using.

4) Make sure that the cables are inserted all the way into the Mac computer port and that any adapters used are securely connected.

5) Turn on your deck.

6) Review the manual for your deck and make sure the output settings are correct. Manuals are available as part of our [Deck Guide](https://mipops.github.io/dvrescue/sections/deck_guide.html). Notes are also available for unique characteristics, tips and tricks.
7) Check your connection is successful by opening the dvrescue and running checking list of detected devices in capture mode.


## Troubleshooting

If your deck does not show up, try the following troubleshooting. A video guide related to this troubleshooting is available [here](https://youtu.be/7FaZw3RoVbA?t=232).

__Check for Device via QuickTime__
* Open Quicktime
* In the File menu, select “New Movie Recording”
* In the viewer that opens, click on the arrow next to the red record button.
* See if your device is listed in the menu that opens.

If your device does not show up in QuickTime, you should also try the restart steps below, but will likely need to move on to the connections troubleshooting section as well. 

__Restart__
* Turn off the deck and restart your computer.
* Once the computer reboots, turn the deck back on.

If this does not resolve the issue, check the following items with your connection.

__Connections__

* Ensure all of your cables are not bent at a hard angle or straining to reach either the deck or the computer, and are properly supported. All three of these factors can cause cable failure.
* As noted above, make sure that the port and cable you are using both have the Thunderbolt symbol. For some Mac laptops, the symbols are not included above the ports, but you can check the specifics by reviewing the [Apple Port Guide](https://support.apple.com/en-us/HT201736).

__Failed Component__
* Try using a different cable, adapter or deck (changing one variable at a time in order to weed out the part that is not working). The most common issue would be a failed cord or adapter. Unfortunately, Apple adapters seem to have a longer lifespan than other brands at this time.

## Mac Station Examples
<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2012.png"><img alt="mac station example 2012" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2012.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2012_4pin.png"><img alt="mac station example 2012 four pin" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2012_4pin.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2014.png"><img alt="mac station example 2014" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2014.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2014_4pin.png"><img alt="mac station example 2014 four pin" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2014_4pin.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2017.png"><img alt="mac station example 2017" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2017.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2017_4pin.png"><img alt="mac station example 2017 four pin" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2017_4pin.png"></a>

