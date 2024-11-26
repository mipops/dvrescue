---
layout: post
title: Capture
---

This portion of the DVRescue documentation will explain how to capture a DV videotape using DVRescue. For details on how to set-up your transfer station, installing DVRescue and troubleshooting connections, please see the <a href="https://mipops.github.io/dvrescue/sections/dv_transfer_station.html" target="_blank">DV Transfer Station section</a>. For a demonstration of the latest version of the GUI running dvcapture, see one of this <a href="https://drive.google.com/file/d/1kkACq_wFFds1r92vxFxAB6RDe1DQyr0p/view?usp=sharing" target="_blank">version 2024-07-24</a>.

<div class="blockquote">
PLEASE NOTE: HDV tapes are not in the current scope of this project and cannot be captured using DVRescue. For more information, please see this <a href="https://github.com/mipops/dvrescue/issues/53" target="_blank">issue</a> discussing the differences between DV and HDV. 
</div>
<br/>

# How to capture using the DVRescue GUI
Step-by-step instructions for using the DVRescue graphic user interface to capture DV videotape formats. 

<div class="blockquote">
PLEASE NOTE: Currently, the DVRescue graphic user interface (GUI) only supports capture on macOS (not Windows or Linux). The command line interface (CLI) supports capture on Linux and Windows (Windows PCs require a dual boot with Ubuntu to work with dvcapture). <a href="https://github.com/amiaopensource/vrecord/blob/main/Resources/Documentation/dv_info.md" target="_blank">dvcapture in vrecord</a> can also capture DV on Linux and Windows (Windows PCs require a dual boot with Ubuntu to work with dvcapture). The other tools (dvanalysis, dvmerge, and dvpackager) are supported on all three platforms via the GUI and the CLI.
</div>
<br/>

<details markdown=1>
  <summary markdown="span">GUI Navigation</summary> <br />
  
## GUI Navigation

In the “Capture” tab from the left-hand menu, you will see a large box in the center of the screen showing either your capture window:

![dvcapture_02_NEW](https://github.com/user-attachments/assets/1abbbc94-682b-4232-a814-8aa24f2dcf4a)

Or (if you do not have a deck connected or if the deck is off) the following message:

![no-deck-detected_small](https://github.com/user-attachments/assets/d5d3aabc-c84b-4680-b6f0-b777e3674038)

If the deck is successfully connected, the device identifier number, make and model of your DV deck should appear above the capture playback window. If you have more than one deck connected, multiple players will appear. Please note: If you have two decks of the same make and model connected, you will need to determine the corresponding identifier for each (the number listed before the make and model of each deck). 
   - To determine the device identifier numbers for two or more decks of the same make and model, turn all but one of the decks off.
   - Check the name of the device in the dvrescue GUI. 
   - Note the device identifier number displayed. 
   - Repeat this for all decks of the same make and model.

Once you have specified a file location and name for your capture, the file name will also appear in the upper right hand corner of the corresponding deck being used to capture it.

The player buttons below the capture window can control the deck, and the icon to the right of the player buttons indicates the current status of the deck.

<img width="1422" alt="dvcapture_07" src="https://github.com/user-attachments/assets/b6fffc81-feca-4c71-b01c-ca7564c6aa8c">

Counters underneath the player buttons track video by frame number, timecode, and the actual record time that has elapsed.

The graph underneath the counters tracks frames with error concealment for audio and video. If you hover your cursor over the graph, a tooltip will display the running total of frames containing errors.

![dvcapture_02B](https://github.com/user-attachments/assets/add62d5f-6b38-434c-9ff9-2c8728df800d)

When a tape is playing, DVRescue will detect the DV format recorded (DV SP, DV LP, DVCAM, DVCPRO, or HDV) and display a pink indicator lamp to the left of the deck control buttons. 

**Please note:** Transferring HDV is not supported by the DVRescue project. Please see <a href="https://github.com/mipops/dvrescue/issues/53" target="_blank">issue #53</a> for more details. 

</details>

<details markdown=1>
  <summary markdown="span">Capture Instructions: DVRescue GUI</summary> <br />

## GUI Capture
  
1. In the dvrescue GUI, click on the “Capture” tab from the list on the left hand side. If you do not have a deck connected or if the deck is off, you will see the following message. <br /> ![no-deck-detected_small](https://github.com/user-attachments/assets/d5d3aabc-c84b-4680-b6f0-b777e3674038) <br /> 

2. Turn on your DV deck. The device identifier number, make and model of the deck should appear above the capture playback window. While a tape is not playing, the screen will read "DECK CONNECTED" on the playback screening. <br /> <img width="1512" alt="Screenshot 2024-08-28 at 6 35 49 PM" src="https://github.com/user-attachments/assets/b47c995d-8cca-4842-a9a6-4a001a003664"> <br /> 

3. If you have more than one deck connected, multiple players will appear in the Capture tab. Please select the one you want to use by clicking on the name of the deck. <br /> <img width="1512" alt="Screenshot 2024-08-28 at 6 24 50 PM" src="https://github.com/user-attachments/assets/2b5391d1-2ee4-43ff-a8a3-5c7b7be08edb"> <br /> 

Please note: If you have two decks of the same make and model connected, you will need to determine the corresponding identifier for each (the number listed before the make and model of each deck). 
   - To determine the device identifier numbers for two or more decks of the same make and model, turn all but one of the decks off.
   - Check the name of the device in the dvrescue GUI. 
   - Note the device identifier number displayed. 
   - Repeat this for all decks of the same make and model.

4. If your deck has a Remote/Local setting, make sure it is set to “Remote” before initiating the capture. Please note that some decks are known to have different behavior. Additional settings for your DV device may apply. To view manuals and settings notes for specific decks, players and camcorders, please see the <a href="https://docs.google.com/spreadsheets/d/1HvNsC110aBH8WjfnNJrF6KlT2STgCT5KRBaf1g6MxGQ/edit#gid=0" target="_blank">DV Deck Guide</a> for deck-specific information.

5. You can use the player buttons to control the deck and preview the content on the tape. <br /> <img width="1422" alt="dvcapture_07" src="https://github.com/user-attachments/assets/df9bd62b-b89f-4db9-9270-d8becab6ca4a"> <br /> 

6. If you would like to enable the deck to rewind and recapture a frame when it encounters an error, click on the Settings. When dvcapture encounters an error, it has the capability to automatically rewind and try to capture the frames containing errors again. You can enable this feature by setting a number of times to rewind and recapture under the "Settings" tab. Click on the “Settings” tab and enter a number in the field (between 0-20, with the default set as zero). We reccomend 3-4 as any more than that can cause damage to the tape and don't result in a better file. Please see the GUI Navigation section for more capture settings options. [more details on Capture settings coming soon.] <br /> <img width="643" alt="Screenshot 2024-07-29 at 3 07 53 PM" src="https://github.com/user-attachments/assets/aab7a6bf-9e64-4d25-ab30-46aed70a6934"> <br /> 

7. When ready, click the record button to initiate the transfer. Please note: For macOS set-ups, if you haven’t previously given DVRescue permission to use your camera, the following message will appear. Click “OK” to allow DVRescue to connect to the deck. <br /> <img width="665" alt="Screenshot 2024-07-29 at 3 06 08 PM" src="https://github.com/user-attachments/assets/adc84bb5-ef19-45d3-8a9d-31e6aed9fbe0"> <br /> 

8. A pop-up window will open. Here you can navigate to the location where you would like to save the files created during capture, including the video file itself and all of the logs. Click the name listed in the field for the “output directory” and then select the directory. 

9. Type in the tape identifier as the file name and click OK. <br /> <img width="1496" alt="dvcapture_05_NEW" src="https://github.com/user-attachments/assets/a0ae0b71-0b4d-4ca5-8d2e-7d31479dfdb2"> <br /> 

10. Deck control will automatically start recording the tape. While it records, you can preview the video in the GUI player. The record time, frame number and record date will display the information being captured from the tape. The graph will populate with audio and video errors detected, as well as frame containing error concealment. <br /> <img width="720" alt="dvcapture-chart" src="https://github.com/user-attachments/assets/173700e9-58ab-4b30-b9ca-d1322ef5e296"> <br /> If you hover your cursor over the graphic it will tell you how many frames have been captured and how many of those frames contain error concealment. <br /> <img width="1512" alt="Screenshot 2024-08-28 at 6 38 57 PM" src="https://github.com/user-attachments/assets/15cb7a67-9e5b-4547-bc0d-69017e123cc9">

11. If you wish to start a second recording, click on the player in the GUI window and follow the steps above to begin recording. You can monitor all of your simultaneous recordings. <br /> <img width="1512" alt="Screenshot 2024-08-28 at 6 27 23 PM" src="https://github.com/user-attachments/assets/52536742-8996-4271-a8d2-f377f2ed6476"> <br /> 

12. When the recording ends or the deck reaches the end of the tape, DVRescue will stop capturing and automatically initiate rewinding the tape (if this function is available on your deck). If you would like to end the capture before the end of the tape or if there is a large chunk of recorded-over blank space, click “Stop” to end the capture. (DVRescue will continue to register blank tape as content if there is timecode or recording time left over from erasing or recording over previous content).

13. Once you are done capturing files, check the output folder to ensure the file contains all of the content that was recorded and that it plays back normally. 

14. You can click on the “Analysis” tab to perform quality control on the files. All of the files captured during your current session will be populated in the list in the Analysis tab. Please see the <a href="https://mipops.github.io/dvrescue/sections/analysis.html" target="_blank">dvanalysis documentation</a> for instructions and tips for reviewing DV files.  <br />

**Troubleshooting:** If you are experiencing any issues capturing in the DVRescue GUI, please see our <a href="https://mipops.github.io/dvrescue/sections/troubleshooting.htm" target="_blank">troubleshooting</a> on the DVRescue documentation website.

</details>  <br />

# How to capture using the DVRescue CLI
Commands and useage for capturing DV tapes using the dvrescue command line tools (CLI).
<details markdown=1>
  <summary markdown="span">Usage of capture commands</summary> <br />
  
**Basic usage:** <br />
Below is the order you should type the commands for capturing using the CLI.

1. In the Terminal run ```dvrescue --list_devices``` to display the names of the devices connected to your computer. Note the name and device number of the one you want to use for capture. For example: <br /> ```$ dvrescue --list_devices``` <br /> ```0x80458280022265: Panasonic DV [DV]``` 

2. You can either type ```dvrescue``` at the start of the command or include the path to the version of dvrescue you want to use. Example: ```/usr/local/bin/dvrescue```

3. If you only have one device connected, dvrescue will automatically select it. If you have more than one device connected, add the device number ```device://0x[device#]``` (for the corresponding deck you want to use for capture). Example (using the deck listed in step 1): ```dvrescue device://0x0x80458280022265``` 

4. If you want dvrescue to rewind and recapture when it encounters an issue with a frame type ```-y --rewind-count``` and the number of attempts you want it to make before it moves on. Example if you wanted dvrescue to rewind and recapture 3 times: ```-y --rewind-count 3```

5. Type ```-m``` to note you want to create a file.

6. Type the files name. Add a space. Example: ```filename.dv```

7. Type ```-m -``` to note you want to merge the multiple capture attempts for problematic frames (dvrescue will automatically rewind and try to capture the frame again when it encouters a problematic frame).

8. To open a playback window during capture add ```| ffplay -```

9. The full string to capture to the example device, create a file named "filename", using the rewind and recapture feature (with 3 attempts) and view the playback during capture would be: <br /> ```dvrescue device://0x0x80458280022265 -y --rewind-count 3 -m filename.dv -m - | ffplay -``` <br /> For example:  <br />```dvrescue device://0x800460104ce2bad -y --rewind-count 3 -m /Users/libbyshopfauf/Desktop/12345.dv -m - | ffplay -```  <br />

For more examples of frequently used dvcapture command strings, please see the section below. 

</details>

<details markdown=1>
  <summary markdown="span">Examples of frequently used capture commands</summary> <br />

**Capture from a specific device:** <br />
```dvrescue device://0xdevicename -m /pathtofile/filename.dv``` <br />
For example: <br />
```dvrescue device://0x800460104ce2bad -m /Users/libbyshopfauf/Desktop/12345.dv```

**Capture from a specific device and show a preview window:** <br />
```dvrescue device://0xdevicename -m /pathtofile/filename.dv -m - | ffplay -``` <br />
For example: <br />
```dvrescue device://0x800460104ce2bad -m /Users/libbyshopfauf/Desktop/12345.dv -m - | ffplay -```

**Capture from a specific device using rewind, recapture and merge (you can only specify a number of times to rewind and recapture if you specify a device):** <br />
```dvrescue device://0xdevicename -y --rewind-count 3 -m filename.dv -m -```  <br />
For example: <br />
```dvrescue device://0x800460104ce2bad -y --rewind-count 3 -m /Users/libbyshopfauf/Desktop/12345.dv -m -```

**Capture from a specific device using rewind, recapture and merge (3 attempts), show a preview window (with only one deck connected):** <br />
```dvrescue device://0xdevicename -y --rewind-count 3 -m filename.dv -m - | ffplay -```  <br />
For example: <br /> 
```dvrescue device://0x800460104ce2bad -y --rewind-count 3 -m /Users/libbyshopfauf/Desktop/12345.dv -m - | ffplay -```

</details>

<details markdown=1>
  <summary markdown="span">Full list of capture commands</summary>
  
All of these flags and options can also be viewed by typing ```dvrescue -h``` into the command line window.

    --help, -h
        Display this help and exit.

    --version
        Display DVRescue version and exit.

    --cc-format value
        Set Closed Captions output format to value.
        value can be 'scc', 'screen', 'srt'.
        If there is more than one instance of this option,
        this option is applied to the next --cc-output option.

    --cc-output value | -c value
        Store Closed Captions output to value (file name).
        File extension must be the format name (see above)
        if --cc-format is not provided.
        if content is different between Dseq and/or has more than 1 field,
        extension is prefixed by 'dseq%dseq%.' and/or 'field%field%.'.
        There can be more than one instance of this option.

    --cc-tc value
        Set Closed Captions output start time code to value.
        Used for SCC output.
        value format is HH:MM:SS;FF, or 'dv' (for DV first frame time code).

    --webvtt-output value | -s value
        Store WebVTT output to value (file name).

    --xml-output value | -x value
        Store XML output to value (file name).

    --merge value | -m value
        Merge all input files into value (file name),
        picking the best part of each file.

    --merge-log value
        Store merge log to value (file name).

    --merge-output-speed
        Report and merge frames having speed not meaning normal playback
        Is default (it will change in the future).

    --merge-ignore-speed
        Do not report and do not merge frames having speed not
        meaning normal playback.

    --merge-output-concealed
        Report and merge frames having all blocks concealed.
        Is default (it will change in the future).

    --merge-ignore-concealed
        Do not report and do not merge frames having all blocks concealed.

    --merge-log-missing
        Report frames considered as missing (due to time code jump etc).
        Is default if information output format is not CSV.

    --merge-hide-missing
        Do not report frames considered as missing (due to time code jump etc).
        Is default if information output format is CSV.

    --merge-log-intermediate
        Display additional lines of information
        about intermediate analysis during files merge.
        Is default if information output format is not CSV.

    --merge-hide-intermediate
        Hide additional lines of information
        about intermediate analysis during files merge.
        Is default if information output format is CSV.

    --csv
        Output is in CSV format rather than human readable text.

    --caption-presence-change value
        Split XML "frames" elements when there is a caption presence
        change (value = "y") or do not split (value = "n").
        Default is "n".

    --verbosity value | -v value
        Verbosity of the output set to value:
        0: no output.
        5: summary only.
        7: information per frame if there is a problem + summary.
        9: information per frame + summary.

    --timeout value
        Time out limit for the device or pipe input ("-" file name) set to value (in seconds)

    --capture
        Launch capture.
        Is the default if no --cmd option is provided.
        Usable only if input is a device.

    --in-control
        Include an integrated command line input for controlling the input.
        Usable only if input is a device.

    --list_devices
        List detected devices and their ID.

    --list_devices_json
        List detected devices and their ID (JSON output).

    --status
        Provide the status (playing, stop...) of the input.
        By default device://0 is used.
        Usable only if input is a device.

    --cmd value
        Send a command to the input.
        By default device://0 is used.
        Usable only if input is a device.
        value may be:
        play      Set speed to 1.0 and mode to play.
        srew      Set speed to -1.0 and mode to play.
        stop      Set speed to 0.0 and mode to no-play.
        rew       Set speed to -2.0 and mode to play.
        ff        Set speed to 2.0 and mode to play.

    --foreground
        With --cmd or --speed, wait for the device to stop.
        Usable only if input is a device.

    --mode value
        Send a command to the input with the specified mode.
        By default device://0 is used.
        By default value is n if speed is 0 else p.
        Usable only if input is a device.
        value may be:
        n         Set mode to no-play.
        p         Set mode to play.

    --speed value
        Send a command to the input with the specified speed (float).
        By default device://0 is used.
        By default value is 0 if mode is no-play else 1.
        Usable only if input is a device.

    --rewind-count value
        Automatically rewind to last good frame and capture again,
        value times.
        Usable only if input is a device.

    --rewind
        Same as --rewind-count 1

    --rewind-basename value
        Base name of files storing buggy frames per take
        Default is output file name.

</details>  <br />

# How to capture using vrecord
To use dvcapture in vrecord, please see the <a href="https://github.com/amiaopensource/vrecord/blob/main/Resources/Documentation/dv_info.md" target="_blank">documentation</a> on the vrecord GitHub repository. 

# Capture alternative for Windows PCs

<details markdown=1>

  If you are running into trouble capturing on Windows, don't have the necessary setup, or prefer not to use the command line, <a href="https://github.com/mipops/dvrescue/issues/897#issuecomment-2264765485" target="_blank"> one user on GitHub has shared a workaround</a>.

1. Use the old WinDV tool to capture the tape to an AVI file. Settings are as follows:
    1. Capture to Type 1 AVI file
    2. Discontinuity threshold: 0 (always capture to a single file; ignore timecode discontinuities)
    3. Max AVI size: 1000000 (the highest possible value; this is several hours of NTSC video)
    4. Every N-th frame: 1
2. Unwrap the avi file using FFmpeg to a bare DV file:<br />
  `ffmpeg -nostdin -i <name of input AVI file> -map 0:v:0 -c copy -f rawvideo <name of output DV file>.dv`
3. Now you have a bare DV file that you can load into DVRescue for analysis, merging, packaging, etc.

</details>  <br />
