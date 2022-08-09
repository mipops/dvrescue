---
layout: post
title: Hardware Setup 
---


## Equipment Set-up (Thunderbolt)

1) Using the Cable Guide, determine the types of cables, adapters and power supply needed to connect your specific deck to the generation of Thunderbolt available on your computer. Ensure that any Thunderbolt cables are compatible with data transfer (vs. for display only).

2) Plug your deck into its power supply and into a power outlet. Make sure that the cables are inserted all the way into the output port on the back of the deck.

3) Connect your deck to the computer using the required cable (and adapters if needed). For details, see the images below for various ways to set-up the connection based on the input ports available on your computer and the DV IN/OUT on the deck you are using.

4) Make sure that the cables are inserted all the way into the computer's port and that any adapters used are securely connected.

5) Turn on your deck.

6) Review the manual for your deck and make sure the output settings are correct. Manuals are available as part of our [Deck Guide](https://mipops.github.io/dvrescue/sections/deck_guide.html). Notes are also available for unique characteristics, tips and tricks.
7) Check your connection is successful by opening the dvrescue and running checking list of detected devices in capture mode.

## Equipment Set-up (DV/Firewire capable card)
If your computer does not have Thunderbolt inputs then it is necessary to use a native DV connection. If your computer already has this then just select the appropriate cables and follow steps 2-7 of the above Thunderbolt instructions.

If your computer has neither Thunderbolt nor Firewire connections, then it will be necessary to install an expansion card that can enable Firewire inputs. These are readily available and can be found for purchase by searching for "Firewire PCIe cards." When purchasing, make sure that the type of card you are buying is something that can be installed in your target computer - some computers (such as Macs) lack expansion capabilities. After following the manufacturers instructions for card installation, select the appropriate cables and follow steps 2-7 of the above Thunderbolt instructions.

## Troubleshooting

If your deck does not show up, try the following troubleshooting. A video guide related to this troubleshooting is available [here](https://youtu.be/7FaZw3RoVbA?t=232).

__Check for Device via QuickTime__
* Open Quicktime
* In the File menu, select “New Movie Recording”
* In the viewer that opens, click on the arrow next to the red record button.
* See if your device is listed in the menu that opens.

If your device does not show up in QuickTime, you should also try the restart steps below, but will likely need to move on to the connections troubleshooting section as well.

__Check for Device via DVGrab (Linux)__
On computers running flavors of Linux the command line program can be used to test your deck connection. This program can be installed via package managers (for example with `sudo apt install dvgrab` on Ubuntu). Once installed, running the command `dvgrab` in a terminal should trigger DVGrab to attempt a capture from detected deck. If this is unsuccessful you will need to move on to additional troubleshooting steps below.

__Restart__
* Turn off the deck and restart your computer.
* Once the computer reboots, turn the deck back on.

If this does not resolve the issue, check the following items with your connection.

__Connections__

* Ensure all of your cables are not bent at a hard angle or straining to reach either the deck or the computer, and are properly supported. All three of these factors can cause cable failure.
* As noted above, make sure that the port and cable you are using both have the Thunderbolt symbol. For some Mac laptops, the symbols are not included above the ports, but you can check the specifics by reviewing the [Apple Port Guide](https://support.apple.com/en-us/HT201736).

__Failed Component__
* Try using a different cable, adapter or deck (changing one variable at a time in order to weed out the part that is not working). The most common issue would be a failed cord or adapter. Unfortunately, Apple adapters seem to have a longer lifespan than other brands at this time. For a step-by-step guide to ensure you determine the correct variable, see the following section. For most of these tips, the assumption is that you have additional cables, adapters and Firewire devices. It is our recommendation that when you purchase a cable or adapter that you buy at least two (or more) at a time to ensure that you have a replacement for testing and in the event of cable failure.

* Try another device:
  - Try connecting your Firewire cable to either a different deck or another Firewire compatible device (such as a legacy hard drive or camera) and see if the device shows up (using the steps “Establishing Deck to Computer Connection” and “Restarting” sections above). 
  - If the second device shows up, there may be an issue with the first deck you were attempting to use (likely the FireWire output is broken).
  - Try connecting the first device again. If it shows up, you are good to go. If it fails to connect, we would recommend having the device inspected for possible damage. 
  - If the second device does not show up, move onto the next section.
* Try another cable:
  - Preferably, using a brand new cable, try connecting your deck to the computer (either with or without adapters depending on your set-up).
  - Using the steps listed in the “Establishing Deck to Computer Connection” and “Restarting” sections above, see if the device shows up in dvrescue, QuickTime or the Terminal.
  - If the deck shows up, you are ready to capture.
  - If the deck does not show up, move on to the next section.
* Try another adapter:
  - If you are using more than one adapter, follow these steps for each one, switching out one at a time.
  - Preferably, using a brand new adapter, try connecting your deck to the computer.
  - Using the steps listed in the “Establishing Deck to Computer Connection” and “Restarting” sections above, see if the device shows up in either dvrescue, QuickTime or the Terminal.
  - If the deck shows up, you are ready to capture.
  - If the deck does not show up and you are using more than one adapter, try the switching out the second one.
  - If none of this works, there might be a problem with your computer’s FireWire port or drivers.

## Thunderbolt Equipped Station Examples
<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2012.png"><img alt="mac station example 2012" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2012.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2012_4pin.png"><img alt="mac station example 2012 four pin" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2012_4pin.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2014.png"><img alt="mac station example 2014" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2014.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2014_4pin.png"><img alt="mac station example 2014 four pin" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2014_4pin.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2017.png"><img alt="mac station example 2017" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2017.png"></a>

<a href="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2017_4pin.png"><img alt="mac station example 2017 four pin" src="{{ site.baseurl }}/images/macOS_Transfer-Station_Setup_2017_4pin.png"></a>

