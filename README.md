# DVRescue

Archivist-made software that supports data migration from DV tapes into digital files suitable for long-term preservation.

The final documentation website is being developed, and is available at: https://mipops.github.io/dvrescue/

For most current development docs check out the
[google drive documentation](https://drive.google.com/drive/u/1/folders/14sKKGQ3pr5g43Po5PNwBOTKINGqHETF-)

DVRescue was presented at the Association of Moving Image Archivists 2019 conference. Slides from the presentation are available [here](https://docs.google.com/presentation/d/1JFYHaKMZWHd8LFOVWmGuxdLNgFOiSXcoSxCMB-TXCvA/edit?usp=sharing).

A status update of DVRescue was presented at the Association of Moving Image Archivists 2020 conference. The presentation video can be viewed [here](https://youtu.be/YGPIqJ4_ssI).

## How to download

DVRescue is available for all major operating systems (macOS, Windows, and Linux). Installers can be downloaded here: https://mediaarea.net/DVRescue  

### Daily snapshots

Daily snapshots of the compiled project are available here: https://mediaarea.net/download/snapshots/binary/dvrescue/

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

Copyright (c) 2019-2020, Moving Image Preservation of Puget Sound.
 
Third party libraries Copyright (c) 2012-2020 MediaArea.net SARL.
