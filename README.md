# DVRescue

Archivist-made software that supports data migration from DV tapes into digital files suitable for long-term preservation.

## Documentation and resources

The final [documentation website](https://mipops.github.io/dvrescue/) is under development as the project progresses and updated as tools are finalized.

For the most current development instructions check out the
[Google Drive draft documentation](https://drive.google.com/drive/u/1/folders/14sKKGQ3pr5g43Po5PNwBOTKINGqHETF-).

DVRescue project status updates, demonstrations and discussions have been presented at various archival conferences. For a full list of recorded presentations, please see the [project documentation page](https://mipops.github.io/dvrescue/sections/01_introduction.html).

## How to download

DVRescue is available for all major operating systems (macOS, Windows, and Linux). Installers can be downloaded [here](https://mediaarea.net/DVRescue). Currently, the GUI is fully functional on macOS. Work continues on the GUI for Windows and Linux. For more information, please see the [issue tracker](https://github.com/mipops/dvrescue/issues).

Please see the documentation for more information on [installation](https://mipops.github.io/dvrescue/sections/installation.html) and [hardware setup](https://mipops.github.io/dvrescue/sections/dv_transfer_station.html) including troubleshooting tips.

### Daily snapshots

Daily snapshots of the compiled project are available for the command line tools ([CLI](https://mediaarea.net/download/snapshots/binary/dvrescue/)) and graphic user interface ([GUI](https://mediaarea.net/download/snapshots/binary/dvrescue-gui/)).

### Homebrew

The latest stable releases of DVRescue and associated tools can also be downloaded via [Homebrew](https://brew.sh), a package manager for Mac (or Linux), using the following commands:

```
brew tap mediaarea/homebrew-mediaarea
brew install dvrescue
```

This Homebrew formula will install the following tools: `dvloupe dvmap dvpackager dvplay dvrescue dvsampler`.

## How to use

### DVRescue

`dvrescue` can be run on the command line by calling the program and passing in a filepath, like so:

`dvrescue path/to/my/video.dv`

### Additional tools

Within this repository are additional tools used to supplement DVRescue. For demonstrations on how to use the command line tools, please see the AMIA 2020 presentation video [here](https://youtu.be/YGPIqJ4_ssI) and related blog posts [here](https://mipops.tumblr.com/).

They are:

**dvloupe** A script that reports on a single frame with color coding and related data.

**dvmap** A script that makes the arrays used in dvloupe. (Not very useful on its own)

**dvpackager** A script that rewraps one DV stream into two MOV files. Used for splitting DV files when the significant characteristics change.

**dvplay** A script that plays back and visualizes the DV errors as a stack of images. Running with the `-x` flag will produce JPEGs instead of just playing them. Requires FFplay. 

**dvrescue.xsd** This file can be used to validate a DVRescue XML output. It also contains definitions of the DVRescue parameters.

**dvsampler** A script that downloads or creates DV files to use as samples. Requires FFmpeg.

**dvmerge** A script that takes multiple transfers of the same tape containing errors and combines them to create one file with the best information available for each problematic frame. dvmerge is part of dvrescue. See `dvrescue -h` on a recent build.

**dvguidance** A collection of additional resources including documentation, video demostrations, and solutions for how the operator may improve the transfer as guided by DVRescue. dvguidance resources can be found [here](https://mipops.github.io/dvrescue/).


## Copyrights

Copyright (c) 2019-2023, Moving Image Preservation of Puget Sound.
 
Third party libraries Copyright (c) 2012-2023 MediaArea.net SARL.
