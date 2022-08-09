---
layout: post
title: DVRescue Schema Documentation
---

# DVRescue XML Schema Documentation

The schema used in DVRescue can be found [here](https://github.com/mipops/dvrescue/blob/main/tools/dvrescue.xsd)

### creator:
References the name and versioning of libraries used by the application listed as creator.
  - _program_: name of program
  - _version_: version of program
  - _library_: version of library

### media:
A media element refers to an input to dvrescue, commonly a file which includes dv data or a piped input.
  - _ref_: Reference to the original file path.
  - _format_: Format of the file, for example DV, AVI, MPEG-4.
  - _size_: File size, in bytes.
  - _error_: Provides an error message is there is an issue parsing the file.

### frames:
As the qualities of a dv stream can change from frame to frame, the frames element groups together frames of like qualities. If a dv frame contains a specific quality that is different than that of the previous frame (such as a change from 4/3 aspect ratio to 16/9 or a change from 48000 Hz audio to 32000 Hz audio), then a new frames element will start.
  - _scan_type_: String representing the interlacement pattern of the frame. Options are:

        T: top, 
        TT: top field only, 
        B: bottom, 
        BB: bottom field only,
        P: progressive

  - _count_: Frame count within the current frames sequence.
  - _pts_: Presentation timestamp for start of sequence.
  - _end_pts_: Presentation timestamp for end of sequence.
  - _size_: Dimensions of frame represented as width x height in pixels.
  - _video_rate_: Frame rate.
  - _chroma_subsampling_:  Chroma subsampling value represented in three part ratio notation.
  - _aspect_ratio_: Frame aspect ratio represented in fractional notation.
  - _audio_rate_: Frame audio rate represented in full integer notation.
  - _channels_:  Number of audio channels, between 0-8.

            DV25: 0-2
            DV50: 0-4
            DV100: 0-8

  - _captions_: Values of 'y' and 'n' indicate whether there is or is not the presence of EIA-608 closed captioning packs with the VAUX section of the DV frames. 'p' when some but not all frames have captions; in that case frame/@cap contains information about begin and end of the closed captioning packs presence.
  - _no_sourceorcontrol_aud_: Boolean that indicates if no audio source and no audio source control pack is present in audio but at least 1 pack

### frame
A frame element holds notable information about frames that have been identified to include errors.
  - _n_: The number of the DIF sequence within the frame starting from zero.
  - _pos_: Position (byte offset) of the frame.
  - _pts_: Presentation timestamp for frame element.
  - _abst_: The Absolute Track number for frame element, which references its corresponding tape position.
  - _abst_r_: Boolean that indicates if absolute track number is repeating.
  - _abst_nc_: Boolean that indicates if absolute track number is non-consecutive.
  - _tc_:  Timecode.
  - _tc_r_: Boolean that indicates if timecode is repeating.
  - _tc_nc_: A value indicating if timecode is non-consecutive.

            1: The timecode is non-continuous and the current value is greater than the previous value.
            2: The timecode is non-continuous and the current value is less than the prev value.

  - _rdt_: Recorded Date Time.
  - _rdt_r_: Boolean that indicates if recorded date time is repeating.
  - _rdt_nc_: A value indicating if recorded date time is non-consecutive.

        1: The recorded date time is non-continuous and the current value is greater than the previous value.
        2: The recorded date time is non-continuous and the current value is less than the prev value.

  - _rec_start_: Recording start.
  - _rec_end_: Recording end.
  - _seqn_: Sequence number. DVRescue represents this four bit value in hexadecimal. For 525-60 systems values of 0, 2, 4, 6, 8, and A imply the frame is a colour frame A and values of 1, 3, 5, 7, 9, and B imply the frame is a colour frame B. For 625-50 systems values of 0, 4, and 8 imply the frame is field 1 and field 2, values of 1, 5, and 9 imply the frame is field 3 and field 4, values of 2, 6, and A imply the frame is field 5 and field 6, and values of 3, 7, and B imply the frame is field 7 and field 8. Values of C, D, and E are not used and a value of F implies that there is no sequence number information. Generally sequence numbers are recorded in a repeating pattern of 0 through B sequentially.
  - _seqn_r_: Boolean that indicates if the sequence number is repeating.
  - _seqn_nc_: Boolean that indicates if the sequence number is non-consecutive.
  - _caption-parity_: A value of 'mismatch' indicates that a EIA-608 closed captioning pack is present within the VAUX section of the frame, but that at least one of the contained captioning values fails its parity check.
  - _no_pack_: Boolean that indicates if no pack is present in subcode/video/audio.
  - _no_pack_sub_: Boolean that indicates if no pack is present in subcode but at least 1 pack is present is video or audio.
  - _no_pack_vid_: Boolean that indicates if no pack is present in video but at least 1 pack is present is subcode or audio.
  - _no_pack_aud_: Boolean that indicates if no pack is present in audio but at least 1 pack is present in subcode or video.
  - _no_sourceorcontrol_vid_: Boolean that indicates if no video source and no video source control pack is present in video but at least 1 pack is present in video.
  - _no_sourceorcontrol_aud_: Boolean that indicates if no audio source and no audio source control pack is present in audio but at least 1 pack is present in audio.
  - _full_conceal_: Boolean that indicates that all video information in frame consists of error concealment, and all audio information consists of audio error codes.
  - _full_conceal_vid_: Boolean that indicates that all video information in frame consists of error concealment.
  - _full_conceal_aud_: Boolean that indicates that all audio information in frame consists of error codes.
  - _conceal_aud_type_: See audType@t.
  - _conceal_aud_value_: See audType@v.
  - _captionType_: When frames/@captions is 'p' (partial), contains information about closed captioning packs presence.

        'on' at the first (in this 'frames' element) frame starting a closed captioning packs stream.
        'off' at the first frame without captioning packs when previous frames have it.


### dseq
  - _n_: The number of the DIF sequence within the frame starting from zero. An NTSC DV25 frame will have ten DIF sequences (0-9) and a PAL DV25 frame will have twelve (12) DIF sequences (0-11).

### sta
 - _t_:  Documents a type of non-zero STA value present in the frame (see table 26 of s314m). For convenience:
          
          0 = No error, what a nice DV macroblock.
          2 = Replaced a macroblock with the one of the same position of the previous frame (guaranteed continuity).
          4 = Replaced a macroblock with the one of the same position of the next frame (guaranteed continuity).
          6 = A concealment method is used but not specified (guaranteed continuity).
          7 = Error with an error code within the macro block. O_O
         10 = Same as 2 but continuity is not guaranteed.
         12 = Same as 4 but continuity is not guaranteed.
         14 = Same as 6 but continuity is not guaranteed.
         15 = Error with unknown position.

 - _n_:  A count of this particular type of STA value within the frame or dseq. Since STA values only apply to video DIF blocks, the maximum value for a DV25 NTSC frame is 1350 (135 video-DIF-blocks * (10 DIF-sequences / NTSC-frame)) and the maximum value for a DV25 is 1620 (135 video-DIF-blocks * (12 DIF-sequences / PAL-frame)).
 - n_even: A count of this particular type of STA value within the even-numbered DIF sequences of the frame. @n minus @n_even would provide the count of this particular type of STA value within the odd-numbered DIF sequences of the frame. A discrepancy between the counts within even and odd DIF sequences can indicate that the associated error is from the playback device of the source tape rather than damage to the source tape. The range is 0-675 for NTSC DV25 and 0-810 for PAL DV25.

### aud
 - _t_: Documents the type of audio errors within the frame.

        1 = An audio DIF block is filled with 0x8000 which indicates an invalid audio sample.
        2 = An audio DIF block is filled with identical value not 0x8000 which indicates an invalid audio sample (not standard).

 - _v_: In case @t is 2, the repeated values, separated by a space; as the test is per block, there may be several values; values may be in hexadecimal (in that case, they are prepended with 0x).
 - _n_: A count of this particular type of audio error within the frame or DIF sequence number (dseq). The maximum value for a DV25 NTSC frame is 90 (9 audio-DIF-blocks * (10 DIF-sequences / NTSC-frame)) and the maximum value for a DV25 is 108 (9 audio-DIF-blocks * (12 DIF-sequences / PAL-frame)).
 - _n_even_: A count of this particular type of audio error within the even-numbered DIF sequences of the frame. @n minus @n_even would provide the count of this particular type of audio error within the odd-numbered DIF sequences of the frame. A discrepancy between the counts within even and odd DIF sequences can indicate that the associated error is from the playback device of the source tape rather than damage to the source tape. The range is 0-45 for NTSC DV25 and 0-54 for PAL DV25.



