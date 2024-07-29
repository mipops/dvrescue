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
<img width="603" alt="Screenshot 2024-07-29 at 11 56 07 AM" src="https://github.com/user-attachments/assets/1ce94115-f6fc-467f-9b22-cb4e80763909">

2.) You can either type ```dvrescue``` at the start of the command or include the path to the version of dvrescue you want to use. Example: ```/usr/local/bin/dvrescue```

4.) If you only have one device connected, dvrescue will automatically select it (device 0). If you have more than one device connected, add the device number ```device://1``` (for the corresponding deck you want to use for capture). Example (using the deck listed in step 1): ```dvrescue device://0``` 

5.) If you want dvrescue to rewind and recapture when it encounters an issue with a frame type ```--rewind-count``` and the number of attempts you want it to make before it moves on. Example if you wanted dvrescue to rewind and recapture 3 times: ```--rewind-count 3```

6.) Type ```-m``` to note you want to create a file.

7.) Type the files name. Add a space. Example: ```filename.dv```

8.) Type ```-m``` to note you want to merge the multiple capture attempts for problematic frames (dvrescue will automatically rewind and try to capture the frame again when it encouters a problematic frame).

9.) To open a playback window during capture add ```| ffplay -```

</details>


<details markdown=1>
  <summary markdown="span">Examples of frequently used capture commands</summary> <br />
  
**Basic capture (with only one deck connected)**<br />
```dvrescue -m filename.dv```

**Capture to a specific device:**<br />
```dvrescue device://0 filename.dv```

**Capture using rewind, recapture and merge:**<br />
```dvrescue --rewind-count 3 -m filename.dv -m```

**Capture using rewind, recapture and merge (3 attempts), show a preview window (with only one deck connected):**<br />
```dvrescue --rewind-count 3 -m filename.dv -m - | ffplay -```
</details>

<details markdown=1>
  <summary markdown="span">Full list of capture commands</summary>
  
All of these flags and options can also be viewed by typing dvrescue -h into the command line window.

**--capture** = Launch capture.<br />
Is the default if no --cmd option is provided.<br />
Usable only if input is a device.<br />
Needs to be followed by the path and name for the output file.

**--in-control** = Include an integrated command line input for controling the input.<br />
Usable only if input is a device.

**--list_devices** = List detected devices and their indices.

**--statusl** = Provide the status (playing, stop...) of the input.<br />
By default device://0 is used.<br />
Usable only if input is a device.

**--cmd [value]** = Send a command to the input.<br />
By default device://0 is used.<br />
Usable only if input is a device.	<br />
Value may be:<br />
play = Set speed to 1.0 and mode to play.<br />
srew = Set speed to -1.0 and mode to play.<br />
stop = Set speed to 0.0 and mode to no-play.<br />
rew = Set speed to -2.0 and mode to play.<br />
ff = Set speed to 2.0 and mode to play.

**--mode [value]** = Send a command to the input with the specified mode.<br />
  By default device://0 is used.<br />
  By default value is n if speed is 0 else p.<br />
  Usable only if input is a device.<br />
  Value may be:<br />
  n = Set mode to no-play.<br />
  p = Set mode to play.<br />

**--speed [value]** = Send a command to the input with the specified speed (float).<br />
By default device://0 is used.<br />
By default value is 0 if mode is no-play else 1.<br />
Usable only if input is a device.

****--rewind-count [value]** = Automatically rewind to last good frame and capture again, value times.<br />
Usable only if input is a device.

**--rewind** = Same as --rewind-count 1

**--rewind-basename [value]** = Base name of files storing buggy frames per take<br />
Default is output file name.
</details>

## How capture using vrecord
To use dvcapture in vrecord, please see the [documentation]([url](https://github.com/amiaopensource/vrecord/blob/main/Resources/Documentation/dv_info.md)) on the vrecord GitHub repository. 
