---
layout: post
title: Glossary
---

# DVRescue Glossary Terms

This glossary has incorporated terms used at MIPoPS with the [Preservation Glossary](https://cool.culturalheritage.org/byorg/bavc/bavcterm.html) published by the [Bay Area Video Coalition](https://bavc.org/), [The AV Artifact Atlas](http://www.avartifactatlas.com), and with contributions from Libby Hopfauf and Jen Zook.

## Adapter
A small piece of computer hardware that connects to a port on another device to provide it with additional functionality, or enable a pass-through to such a device that adds functionality. Specifically used here to reference connecting DV decks to computers through various combinations of inputs and outputs to enable a Firewire connection. See also dongle.

## Analog recording
A recording in which continuous magnetic signals are written to a tape that are representations of the voltage signals coming from the recording of the video camera or microphone. Analog signals stored on tape deteriorate with each copy or generation; in contrast see digital.

## Analog-to-digital
The process in which a continuous analog signal is quantized and converted to a series of binary integers.

## Arbitrary bit inconsistency
This is noted when the arbitrary bits within a frame are not consistent. As noted this information is ‘arbitrary’, but can indicate issues with the frame, such as when data from two different frames is mixed into a single frame (from recording over recordings or glitchy playout).


## Archival format
A video format that provides reliable playback, without information loss. The format should be a current (as opposed to obsolescent) professional one supported by the industry. At present archival video material is typically stored on magnetic tape, however in the near future computer-based storage is likely to become an option for archives. The advantage of uncompressed digital formats over analog formats is that they can be copied without generational loss. For this reason many archives are using digital formats for creating their archival masters. Ideally these formats should be uncompressed, component formats; however, for practical and cost reasons, suitable archival formats will change as older formats become obsolete and are no longer supported. Ideally, archival master material is transferred onto new stock every 5-7 years and at this point a decision should be made about whether it is necessary to move to a new format as well. An archival format is therefore one that can be migrated onto new stock and new formats without the loss or distortion of information.

## Artifact
An undesirable picture element in a video image, which may naturally occur in the recording process and must be eliminated in order to achieve a high quality image. Most common artifacts are cross-color and cross-luminance. Not to be confused with an artifact as a cultural product. 

## Aspect ratio

## Audio block

## Baking
A process of gently heating damaged videotape in an oven with controlled relative humidity in order to enable playback. As magnetic tape deteriorates the polymer of the binder deteriorates by hydrolysis, resulting in what is typically called sticky shed. Archivists have reported success in baking tapes that are suffering severe sticky shed; however, to date the scientific research has not been done to explain this. The temperature and humidity of the oven must be tightly controlled, as does the time for which a tape is baked. This process is not recommended except in extreme circumstances, as there is a suggestion that it will ultimately speed up the deterioration of the tape, although it might enable playback for remastering. There is unfortunately very little research in this area.

## Bearding
A type of video distortion that appears as black lines extending to the right of bright objects.

## Binder
The polymer used to bind magnetic particles together and adhere them to the tape substrate.

## Bit
Shorthand for binary digit, which has two optional values “0” or “1.” Eight bits means 8 binary digits. There are 256 possible combinations for 8 binary digits and therefore color depth of 8 bits represents 256 (2x2x2x2x2x2x2x2) possible colors. Because each pixel of a video picture contains 3 samples Y’, R-Y’, B-Y’, the possible colors of an 8-bit system would be 16.7 million (256 x 256 x 256). Nowadays archives will be receiving digital material into their collections or will be generating it as part of their preservation program. It is therefore necessary that we understand digital and analog technology.

## Bit rate 
The amount of data transported in a given amount of time, usually defined in Mega (Million) bits per second (Mbps). Bit rate is one way to define the amount of compression used on a video signal.

## Bit error rate (BER)
The percentage of bits that have errors in playback, one possible indicator for deterioration. Playback is never perfect and there are many possible causes of error such as noise, dirt and dust, and drop out. In the binary world of digital data a bit is either correct or incorrect. Since it only has two states, the challenge is to correctly identify whether a bit is correct or not. To enable this the data is therefore coded by adding redundant bits. All systems build in redundancy and error correction mechanisms. Information about bit error rates can refer to the bit error rate prior to error correction or the residual errors after error correction.

## Bitstream
A simple contiguous sequence of binary digits transmitted continuously over a communications path; a sequence of data in binary form.

## Black, or Color Black, Blackburst
A composite color video signal comprised of composite sync, reference burst and a black video signal which is usually at a level of 7.5 IRE (0.05V) above the blanking level. Also refers to fade-to-black between scenes.

## Blanking level
Also known as pedestal, the level of a video signal, which separates the range that contains the picture information from the range that contains the synchronizing information.

## Block

## Blocking
The sticking together or adhesion of successive windings in a tape pack. Blocking can result from deterioration of the binder, storage of tape reels at high temperatures, and/or excessive tape pack stresses.

## Byte

## Byte offset

## Camcorder
A portable combined video camera and video recorder.

## Capture
Transferring or capturing the video, audio and data content from a DV videotape to a file. Since DV already contains digital components, this process is not referred to as “digitizing” as it is for analog videotapes. See [Transfer](#transfer)

## Cassette
A sealed plastic unit containing a length of audiotape, videotape, film, etc. wound on a pair of spools, for insertion into a recorder or playback device.

## Cassette door
A moving, spring-loaded door that protects the tape in a cassette.

## CC
Closed captions (CC) or caption track and subtitling are both processes of displaying text on a television, video screen, or other visual display to provide additional or interpretive information. Both are typically used as a transcription of the audio portion of a program as it occurs (either verbatim or in edited form), sometimes including descriptions of non-speech elements. Other uses have included providing a textual alternative language translation of a presentation's primary audio language that is usually burned-in (or "open") to the video and unselectable.

## CLI
A command-line interface (CLI) processes commands to a computer program in the form of lines of text. The program which handles the interface is called a command-line interpreter or command-line processor (for example macOS uses zsh in the Terminal).

## Codec
Codec (from combining the words coder and decoder) is the hardware or software that compresses video and audio data into a manageable size for viewing, transfer or storage.

## Deck
A type of machine for playing and recording cassettes that does not have a built-in power amplifier or speakers.

## DIF incoherency
A DV decoder relies on a combination of data in DIF sequences to understand the specifications of the codec associated with the file in order to interpret and playback the video and audio. An inconsistency in the data can result in problems during playback, such as missed frames or being out of sync. For addition example see [this MediaArea resource page](https://mediaarea.net/DVAnalyzer/dif-incoherency).

## DIF sequence
In DV, the audio, video, and metadata are packaged into 80-byte Digital Interface Format (DIF) blocks which are multiplexed into a 150-block sequence. DIF blocks are the basic units of DV streams and can be stored as computer files in raw form or wrapped in such file formats as Audio Video Interleave (AVI), QuickTime (QT) and Material Exchange Format (MXF). One video frame is formed from either 10 or 12 such sequences, depending on scanning rate, which results in a data rate of about 25 Mbit/s for video, and an additional 1.5 Mbit/s for audio. When written to tape, each sequence corresponds to one complete track.

A DIF block is a section of code that represents the data transmitted from all of the passes of the head across a tape that make up a frame. Each of these head passes is a DIF sequence.

## Dongle
A small piece of computer hardware that connects to a port on another device to provide it with additional functionality, or enable a pass-through to such a device that adds functionality. Specifically used here to reference connecting DV decks to computers through various combinations of inputs and outputs to enable a Firewire connection. See also [Adapter](#adapter)

## Dropout
Brief signal loss caused by a tape head clog, defect in the tape, debris, or other feature that causes an increase in the head-to-tape spacing. A dropout can also be caused by missing magnetic or digital material. A video dropout generally appears in the following ways:
 - Pink & green
 - Information repeated from prior/successive frames 

## Dropped frame
Drop-frame refers to a system of dropping certain timecodes with the goal of compensating for drift between 60 minutes of timecode 60 minutes of real time caused by the NTSC framerate of 29.97 frames per second. See [Timecode](#timecode)

## DV
A format of digital video (both codecs and physical tapes). See [DV Format Guide](./dv_formats.html) for additional information on the variety of digital videotape formats and the specifics of each.

## Error concealment
Video Error Concealment is one of the most commonly identified preservation issues in the digitization of DV content from tape to file because of the invasive effect the process has on the image. The issues that result from error concealment are often termed as blocking or glitchiness, or described as areas where groups of pixels are shifted out of their expected position.

## Flag

## File container
The file that contains your video and audio streams. It’s common for a container to be called a file extension since they are often seen at the end of file names (e.g. filename.mp4). Popular video containers include .mp4, .mkv, .mov, or .avi, but there are many more. See [wrapper](#wrapper).

## FireWire

## Frame
One of the many still images which compose a moving picture. See [Frame rate](#frame-rate).

## Frame aspect ratio
The same frame size is used for 4:3 and 16:9 frame aspect ratios, resulting in different pixel aspect ratios for fullscreen and widescreen video.

## Frame count
The total number of frames contained in a video file.

## Frame rate
The frequency (rate) at which consecutive images (frames) are captured or displayed.

## GUI
Graphic User Interface (GUI) refers to a version of a set of software tools in a form that allows users to interact with electronic devices through graphical icons and audio indicator such as primary notation, instead of text-based user interfaces, typed command labels or text navigation. An alternative to command-line interfaces (CLIs), which require commands to be typed on a computer keyboard.

## HDV
A distinct format from DV utilizing different codecs for audio and video information than used in standard DV. See [DV Format Guide](./dv_formats.html) for additional information on the variety of digital videotape formats and the specifics of each.

## IEEE 1394 interface
A high-speed computer data-transfer interface used to connect personal computers, audio and video devices, and other professional and consumer electronics. Also called FireWire. All DV cameras that recorded to tape media had a FireWire interface.

## Interlaced scanning
A technique for doubling the perceived frame rate of a video display without consuming extra bandwidth. The interlaced signal contains two fields of a video frame captured consecutively. Video sources that are listed with the letter i are called interlaced. An example of this would be 480i or 1080i. Interlaced video displays even and odd scan lines as separate fields. The even scan lines are drawn on the screen, then the odd scan lines are drawn on the screen. Two of these even and odd scan line fields make up one video frame. This enhances motion perception to the viewer, and reduces flicker by taking advantage of the phi phenomenon. Contrast with [Progressive recording](#progressive-recording).

## intraframe video compression
Intra-frame coding is used in some video [codecs](#codec) (including DV). It refers to compression in which every frame is compressed discretely, and is unreliant on any information stored in different frames for decoding, as is the case with other compression methods.

## Long play (LP)
Long play allows you to record much more information at the expense of quality. It accomplishes this by reducing the speed the tape is run through the recording deck. As the tape speed decreases, the quality also decreases. A MiniDV tape could hold up to 120 minutes of digital video when recorded at LP (long-play) speed.

## Lossy compression
A compression technique that does not decompress digital data back to 100% of the original. Lossy methods can provide high degrees of compression and result in smaller compressed files, but at the expense of a certain amount of the original information being permanently lost.

## Missing packs

## Package

## Parity data
Parity data acts as a secondary data source to be referenced if the primary data is damaged or unreadable. Error concealment relies on data, corresponding pixels from adjacent frames, to replace missing video data and create a new video stream.

## Playback
The act of playing a recording again in order to hear or see something again

## Progressive recording
A format of displaying, storing, or transmitting moving images in which all the lines of each frame are drawn in sequence, as opposed to interlaced scanning. Video sources that are listed with the letter p are called progressive scan signals. Examples of this would be 480p, 720p or 1080p. Progressive scan video content displays both the even and odd scan lines (the entire video frame) on the TV at the same time. See [Interlaced scanning](#interlaced-scanning).

## Raw DV stream
The “original,” or “native,” format of a DV recording.

## Timecode
Timecodes provide a time reference for editing, synchronization and identification. Timecode is a form of media metadata. The invention of timecode made modern videotape editing possible and led eventually to the creation of non-linear editing systems. 
 - SMPTE timecode is a set of cooperating standards to label individual frames of video or film with a timecode. 
 - SMPTE timecode is presented in hour:minute:second:frame format (00:00:00:00) and is typically represented in 32 bits using binary-coded decimal. There are also drop-frame and color framing flags and three extra binary group flag bits used for defining the use of the user bits.

## Timecode inconsistency
DV contains many timecode values per frame (this helps enable the timecode to be viewable while the tape is shuttled). This notes when the timecodes within a frame are inconsistent. This error does not affect video or audio quality but may be indicative of other problems.

## timecode jump


## transfer
Transferring or capturing the video, audio and data content from a DV videotape to a file. Since DV already contains digital components, this process is not referred to as “digitizing” as it is for analog videotapes. See [Capture](#capture).

## Wrapper
See [File container](#file-container)
