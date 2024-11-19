---
layout: post
title: Packaging
---

dvpackager is part of the dvrescue toolkit. It is used to rewrap a raw DV stream into preservation files based on a set of characteristics. For clarification on any terms used in these instructions, see <a href="{{ site.baseurl }}/sections/glossary.html" target="blank">the Glossary</a>. For more details on how dvpackager works, please see <a href="https://mipops.tumblr.com/post/625667299610853376/dvrescue-workshop-1-dvpackager-and-dvloupe" target="blank">our blog post</a>.

# How to package files using the DVRescue GUI

<details markdown=1>
  <summary markdown="span">GUI Navigation</summary>

## GUI Navigation

### Input files

A file list will populate containing any files you have already been working on in other tabs. You can remove any of these with the X and can add more files by clicking the folder with a plus symbol. 
In this section you can also select the segmentation rules, selecting the markers around which you would like to split the file. These different markers, and when they might be useful, are listed below.

### Segmentation Markers
<br />
- **Record start markers:** flags indicating the beginning of a recording.<br />
  This can be helpful for finding the beginning or end of segments that were recorded separately. It can also help with packaging, if you prefer the segments of content in separate files. Some camcorders use both start and stop markers, but most only use one or the other. 

- **Record stop markers:** flags indicating the end of a recording.<br />
  This can be helpful for finding the beginning or end of segments that were recorded separately. It can also help with packaging, if you prefer the segments of content in separate files. Some camcorders use both start and stop markers, but most only use one or the other. 

- **Record time break:** a break in the recording time for a tape.<br />
  This can also help to separate different content into segments.

- **Timecode break:** a break in the timecode for a recording.<br />
  Some creators used different timecodes to mark different scenes or portions of content for footage included in a final project.

- **Audio rate changes:** the rate the audio was recorded at changed, either by the frequency, or the number of channels, or both.<br />
  Audio rate changes can affect the way players handle the audio. The result might be that audio after a change sounds faster or slower. It might also cause audio to be missing for a portion of the file. Segmenting and packaging by audio rate changes is advised in these cases to ensure accurate playback of the audio for all of the content on a tape.

- **Aspect ratio changes:** the aspect ratio changes from 4/3 to 16/9 (or 16/9 to 4/3).<br />
  Similarly to audio rate changes, players cannot always adapt to accommodate both. Segmenting and packaging by aspect ratio changes (or forcing them all to be the same in the packaged file) is advised in these case to ensure accurate playback for both aspect ratios if both were recorded on a single tape.

For more information on segments and how they can be useful, please see <a href="https://drive.google.com/file/d/1Y14qcKvuoZug52fMBoOmw2GbzEjt83ig/view?usp=sharing" target="blank">Andrew Weaver’s breakdown of segmentation suggestions</a>.

### Segmentation Rules

Here, the segments that will be produced using the rules defined above are listed. It shows the frame, timestamp, and timecode; the marker or alert icon that triggered the split; and defines the audio settings and aspect ratio. These details should help in identifying the sections and determining if the file is being split as expected.

### Output Queue
The output queue list all the files that have been processed or are in queue to be processed. The entire file path is listed as well as an icon that shows its current status.

<a href="{{ site.baseurl }}/images/packagerIcons.png"><img alt="Packaging Status Icon Key" src="{{ site.baseurl }}/images/packagerIcons.png"></a>

</details> 

<details markdown=1>
  <summary markdown="span">Merge Instructions</summary>

## GUI Merge

<div class="blockquote">
Note: we recommend previewing files and segmenting options in the Analysis tab prior to packaging in order to perform quality control and review any artifacts, recapture if needed, and merge files to get the best possible copy prior to packaging. Analysis also serves as a good way to test various segmentation rules and determine the best one(s) for each file based on the recording data and content.
</div><br />

1. In the DVRescue GUI, navigate to “Package” from the menu on the left side.
2. Under Input Files, a file list will populate containing any files you have already been working on in other tabs.
  1. You can remove any of these files with the X next to the file name.
  2. You can import other files by clicking the folder icon with a plus symbol or by dragging and dropping the files into the window.
3. If Input Files shows more than one file, make sure to select the one you want to be working with by clicking on it. The file will be highlighted in green when selected. 
4. Below the file list, select any of the segmentation rules you want to use to divide the file.
  1. Read more about these in the section above.
  2. If you are unsure, you may want to first explore more in the Analysis tab.
5. Be sure to click Apply once you have selected the rules.
6. The list in the Segmentation Rules window will show the number of segments that will be output. Make sure this list matches your expectations and no issues are apparent.
7. Choose the file format you would like for the output files: .mov or .mkv.
8. Determine the location to save the files:
  1. If you want to save the packaged files in the same location as the original file, select the “package into same folder” option. This will also keep the same name as the original file with numbers appended as necessary.
  2. If you want to save the files to a separate location, select the “specify path” option and enter the path. Here you may also rename the file.
9. Click the “Add to queue” button. The file will appear in the Output Queue below and will begin processing immediately.
10. While it processes, you can continue working on other files. Repeat steps 3-9 for all files you want to package.
11. As each file processes, an icon to the right will show the status of the file.

</details> 

# How to package files using the DVRescue CLI
Commands and useage for capturing DV tapes using the dvrescue command line tools (CLI).

<details markdown=1>
  <summary markdown="span">Usage of package commands</summary>

## Basic Syntax

The basic packaging syntax looks like this:<br />
  `dvpackager [options] file.dv`

To preview all of the options in order to determine the best way to rewrap a file, import the file into the “Analysis” window and select the “Segment” view.

To generate a list of options or for additional information, run the following command:<br />
  `dvpackager -h`

  </details> 

<details markdown=1>
  <summary markdown="span">Flags and Options</summary>

## Flags and Options

An up-to-date and exhaustive list of flags and options can be viewed by typing `dvpackager -h` into the command line window. However, these are some common ones you may need to use.

By default, dvpackager will split the output files so that each time significant technical characteristics of the DV stream changes (such as aspect ratio, frame rate, and format), a new output file will be written. The following flags adjust the way dvpackager will split the output.

- **-f** = forces dvpackager to ignore changes in significant technical characteristics of the dv stream when splitting the output. 
- **-s** = split the output file at recording start markers.
- **-d** = split the output file at non-consecutive recording timestamps.
- **-t** = split the output file at non-consecutive timecode values.

Example: `-s INPUT.dv` will read `-r INPUT.dv` and rewrap those dv frames into an output file while making one new output file whenever there is a change in significant technical characteristics or frames with recording-start flags.

### Other commonly used options

After the primary command and the input, you can add one or more of the following flags to modify and customize the output.

- **-o [path to directory]** = provide a custom output directory
  - Example: `-o /Users/myusername/MyFolder/DV_Files`
- **-e [extension]** = specify the extension of the container you want to use.
  - This option has been successfully tested with the following wrappers:
    - mkv
    - mov
    - dv
  - If no other specific extension is specified, dvpackager’s default output is mov.
  - Note that using the `dv` option shall simply extract the dv from the file while using the selected options to split the output.
  - Example: `-e mkv`
- **-n** = do not repackage, simply generate a dvrescue xml if one doesn't already exist, and report on what the output files would be.
- **-N <parts>** = List the numbers of the segments that should not be packaged. Multiple segments numbers may be listed as comma delimited
  - Example: `-N ‘1,3’` would output the 2nd segment and any segment after the 3rd.
  - Use `-n` first to list the segments by number.

### Audio resample options
By default, dvpackager will preserve the sampling rate of the input DV stream; however, if the input contains a mixture of 32000 Hz and 48000 hz audio, then dvpackager will resample the packaged audio to 48000 Hz when muxing to a container
- **-3** = Enable this option to keep the sampling rate in its native format.
  - Setting this may result in more output files as each change in sampling rate will require a new output file.
  - Automatically enabled when `-e dv`
- **-4** = Force all audio to be resampled to 48000 Hz

### Aspect Ratio Options
**-a [aspect ratio option]** = Choose a strategy for handling mid-stream changes in aspect ratio. Options include:
- n = Split segments for aspect ratio changes.
- 4 = Do not split segments for aspect ratio changes alone, and if packaging a mix of 4/3 and 16/9 content together, then label it as 4/3 in the container.
- 9 = Do not split segments for aspect ratio changes alone, and if packaging a mix of 4/3 and 16/9 content together, then label it as 16/9 in the container.
- c = Do not split segments for aspect ratio changes alone, and if packaging a mix of 4/3 and 16/9 content together, then label it according to the more common aspect ratio in that segment.

</details> 

<details markdown=1>
  <summary markdown="span">Unpackager</summary>
<br />
dvpackager also has an 'unpackage' mode (currently only available in the CLI):<br />
**-u** = export the dv stream from each provided file into a single dv stream. 

For example, the following command:<br />
`dvpackager -u INPUT_1.mkv INPUT_2.mkv INPUT_3.mkv`<br />
will create one dv stream that contains all the DV of each input file.

</details> 

<details markdown=1>
  <summary markdown="span">Full list of package commands</summary>

All of these flags and options can also be viewed by typing `dvrescue -h` into the command line window.

    By default, dvpackager will split the output files so that each time
    significant technical characteristics of the dv stream change (such as aspect
    ratio, frame rate, audio channel count, or audio sample rate) a new output file
    will be written. The following flags adjust the way dvpackager will split the
    output.

    -s       (split the output file at recording start markers)
    -d       (split the output file at non-consecutive recording timestamps)
    -t       (split the output file at non-consecutive timecode values)

    -o <dir> (provide a custom output directory)
    -O <pattern>
          (specify a pattern for output files. The following variables may be
           used:
          %FILENAME% - will use the filename of the input file without it's
                       extension
          %RECDATE% -  will use the recording date of the first output frame,
                       in YYYY-MM-DD format. If there is no embedded recording
                       date, then 'XXXX-XX-XX' will be used.
          %RECTIME% -  will use the recording date of the first output frame.
                       If there is no embedded recording date, then
                       'XX-XX-XX' will be used.
          %TC% -       will use the timecode value of the first frame or use
                       XX-XX-XX-XX if no timecode is stored in the first frame.
          %PARTNO% -   This is an incrementing number of the output starting
                       from 1.
           The default pattern is "%FILENAME%_part%PARTNO%". The
           extension of the output file is determined by the -e
           setting.)
               
           Note that if the output pattern would generate multiple output files
           with the same name then a 3-digit incrementing number such as "-001"
           will be added to the end of the filename.
    -e <ext> (specify the extension of the container to use. Tested with:
           mkv, mov, dv. Defaults to mov. Note that using the 'dv' option
           shall simply extract the dv from the file while using the
           selected options to split the output.)
    -l <code>(specify the language code to use for the audio tracks.
           If multiple languages are provided with a comma delimiter such as
           'eng,spa,fra,nor' then they are used for each audio track in order.
           Note that usually could be one or two stereo tracks, but if -m is 
           enabled then it is possible for two or four mono tracks, each
           with it's own language code.)
    -L <code>(specify the language code to use for the caption tracks (if
           any).
    -m       (By default, audio will be handled in stereo pairs, enabling this
           option will arrange the audio into tracks with mono channels.)
    -S       (embeds a caption track if there are captions to represent in the
           source DV)
    -Z       (generate a DVRescue technical subtitle file, which contains
           timecode and other frame-based metadata. This is an experimental
           dvrescue subtitle format)
    -n       (do not repackage, simply generate a dvrescue xml if one doesn't
           already exist, and report on what the output files would be)
    -N <parts> (List the numbers of the segments that should not be packaged.
             Multiple segments numbers may be listed as comma-delimited; for
             instance "-N '1,3'" would output the 2nd segment and any segment
             after the 3rd. Use "-n" to list the segments by number.)
    -v       (shows ffmpeg stderr output, otherwise this is hidden)
    -z       (disable colored terminal output)

    -F <path> (provide a custom ffmpeg path)
    -M <path> (provide a custom mediainfo path)
    -D <path> (provide a custom dvrescue path)
    -X <path> (provide a custom xmlstarlet path)
    -x <path> (provide a custom path to a DVRescue XML that corresponds to the
            input file)
    -V <path> (provide a custom path to a DVRescue technical subtitle file that
            corresponds to the input file, ignored if -S is not set)
    -C <path> (provide a custom path to a DVRescue SCC that corresponds to the
           input file, ignored if -S is not set)

    Audio Resample Options

    By default, dvpacakager will preserve the sampling rate of the input DV stream;
    however, if the input contains a mixture of 32000 Hz and 48000 Hz audio, then
    dvpackager will resample the packaged audio to 48000 Hz when muxing to a
    container.

    -3        (Enable this option to keep the sampling rate in its native format.
            Setting this may result in more output files as each change in
            sampling rate will require a new output file. Automatically enabled
            when '-e dv'.)
    -4        (Force all audio to be resampled to 48000 Hz.)
 
    Aspect Ratio Options
    -a <opt>  (Choose a strategy for handling mid-stream changes in aspect ratio.
            Options:
                - n: Split segments for aspect ratio changes.
                - 4: Do not split segments for aspect ratio changes alone,
                     and if packaging a mix of 4/3 and 16/9 content together,
                     then label it as 4/3 in the container.
                - 9: Do not split segments for aspect ratio changes alone,
                     and if packaging a mix of 4/3 and 16/9 content together,
                     then label it as 16/9 in the container.
                - c: Do not split segments for aspect ratio changes alone,
                     and if packaging a mix of 4/3 and 16/9 content together,
                     then label it according the more common aspect ratio in
                     that segment.
            )

    For example, the following command:

    dvpackager -s INPUT.dv

    will read -r INPUT.dv and rewrap those dv frames into an output file while making
    one new output file whenever there is a change in significant technical
    characteristics or frames with recording-start flags.

    dvpackager also has an 'unpackage' mode

    -u       (export the dv stream from each provided file into a single dv stream)

    For example, the following command:

    dvpackager -u INPUT_1.mkv INPUT_2.mkv INPUT_3.mkv

    will create one dv stream that contains all the DV of each input file.

    dvpackager also has a method to express a table of how the file would be
    repackaged given an XML input.
 
    -T <xml> (provide only a table of information about the output files without
           making any, provide the xml as an argument)

    For example, the following command:

    dvpackager -s -a c -T file.dv.dvrescue.xml file.dv

    will show a table of the outputs for file.dv with the provided options (-s -a c).

</details> 
