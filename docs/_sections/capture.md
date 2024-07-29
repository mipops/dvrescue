---
layout: post
title: Capture
---

## DVRescue GUI
Step-by-step instructions for using the DVRescue graphic user interface to capture DV videotape formats. 
<details markdown=1>
  <summary markdown="span">Details</summary>
<iframe class="embed-doc" src="https://docs.google.com/document/d/e/2PACX-1vRi-g_KLJyzX89ZISEYD9v9kH8c-MABKZxjq-mnC9zWFdcLA0FFgoTQU1C2VxMUhlGvvWUwYCxjT1p2/pub?embedded=true"></iframe>
</details>

## How to capture using the DVRescue CLI
Commands and useage for capturing DV tapes using the dvrescue command line tools (CLI).
<details markdown=1>
  <summary markdown="span">Useage of capture commands</summary> <br />
  
**Basic useage:** <br />
Below is the order you should type the commands for capturing using the CLI.

1.) In the Terminal run ```dvrescue --list_devices``` to display the names of the devices connected to your computer. Note the name and device number of the one you want to use for capture. For example: <br />
```
$ dvrescue --list_devices <br />
0x80458280022265: Panasonic DV [DV]
```

2.) You can either type ```dvrescue``` at the start of the command or include the path to the version of dvrescue you want to use. Example: ```/usr/local/bin/dvrescue```

4.) If you only have one device connected, dvrescue will automatically select it. If you have more than one device connected, add the device number ```device://0x[device#]``` (for the corresponding deck you want to use for capture). Example (using the deck listed in step 1): ```dvrescue device://0x0x80458280022265``` 

5.) If you want dvrescue to rewind and recapture when it encounters an issue with a frame type ```--rewind-count``` and the number of attempts you want it to make before it moves on. Example if you wanted dvrescue to rewind and recapture 3 times: ```--rewind-count 3```

6.) Type ```-m``` to note you want to create a file.

7.) Type the files name. Add a space. Example: ```filename.dv```

8.) Type ```-m -``` to note you want to merge the multiple capture attempts for problematic frames (dvrescue will automatically rewind and try to capture the frame again when it encouters a problematic frame).

9.) To open a playback window during capture add ```| ffplay -```

10.) The full string to capture to the example device, create a file named "filename", using the rewind and recapture feature (with 3 attempts) and view the playback during capture would be: ```dvrescue device://0x0x80458280022265 --rewind-count 3 -m filename.dv -m - | ffplay -``` For more examples of frequently used dvcapture command strings, please see the section below. 

</details>


<details markdown=1>
  <summary markdown="span">Examples of frequently used capture commands</summary> <br />
  
**Basic capture (with only one deck connected)** <br />
```dvrescue -m filename.dv```

**Capture to a specific device:** <br />
```dvrescue device://0 -m filename.dv```

**Capture using rewind, recapture and merge:** <br />
```dvrescue --rewind-count 3 -m filename.dv -m -```

**Capture using rewind, recapture and merge (3 attempts), show a preview window (with only one deck connected):** <br />
```dvrescue --rewind-count 3 -m filename.dv -m - | ffplay -```
</details>

<details markdown=1>
  <summary markdown="span">Full list of capture commands</summary>
  
All of these flags and options can also be viewed by typing dvrescue -h into the command line window.

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

If no output file name is provided, XML output is displayed on console output.

libbyshopfauf@Libbys-MacBook-Pro-M2 ~ % 

</details>

## How capture using vrecord
To use dvcapture in vrecord, please see the [documentation]([url](https://github.com/amiaopensource/vrecord/blob/main/Resources/Documentation/dv_info.md)) on the vrecord GitHub repository. 
