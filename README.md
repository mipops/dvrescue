# DVRescue

Archivist-made software that supports data migration from DV tapes into digital files suitable for long-term preservation.

## How to download

Daily snapshots of the compiled project are generously hosted by MediaArea at the following link: https://mediaarea.net/download/snapshots/binary/dvrescue/

Daily builds are provided for all major operating systems (macOS, Windows, and Linux).

## How to use

### DVRescue

`dvrescue` can be run on the command line by calling the program and passing in a filepath, like so:

`dvrescue path/to/my/video.dv`

### Additional tools

Within this repository are additional tools used to supplement DVRescue.

They are:

**dvloupe** A script that repots on a single frame with color coding and related data.

**dvmap** A script that makes the arrays used in dvloupe. (Not very useful on its own)

**dvpackager** A script that rewraps one DV stream into two MOV files. Used for splitting DV files when the significant characteristics change.

**dvplay** A script that plays back and visualizes the DV errors as a stack of images. Running with the `-x` flag will produce JPEGs instead of just playing them. Requires FFplay. 

**dvrescue.xsd** This file can be used to validate a DVRescue XML output. It also contains definitions of the DVRescue parameters.

**dvsampler** A script that downloads or creates DV files to use as samples. Requires FFmpeg.