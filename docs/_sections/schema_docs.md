---
layout: post
title: DVRescue Schema Documentation
---

# DVRescue XML Schema Documentation

The schema used in DVRescue can be found [here](https://github.com/mipops/dvrescue/blob/main/tools/dvrescue.xsd)

### creator:
  - _program_:
  - _version_:
  - _library_:

### media:
  - _ref_:
  - _format_:
  - _size_:

### frames
  - _scan_type_:
  - _count_:
  - _pts_:
  - _end_pts_:
  - _size_:
  - _video_rate_:
  - _chroma_subsampling_:
  - _aspect_ratio_:
  - _audio_rate_:
  - _channels_:
  - _captions_:
  - _no_sourceorcontrol_aud_:

### frame
  - _n_:
  - _pos_:
  - _pts_:
  - _abst_:
  - _abst_r_:
  - _abst_nc_:
  - _tc_:
  - _tc_r_:
  - _tc_nc_:
  - _rdt_:
  - _rdt_r_:
  - _rdt_nc_:
  - _rec_start_:
  - _rec_end_:
  - _seqn_:
  - _seqn_r_:
  - _seqn_nc_:
  - _caption-parity_:
  - _no_pack_:
  - _no_pack_sub_:
  - _no_pack_vid_:
  - _no_pack_aud_:
  - _no_sourceorcontrol_vid_:
  - _no_sourceorcontrol_aud_:
  - _full_conceal_:
  - _full_conceal_vid_:
  - _full_conceal_aud_:
  - _conceal_aud_type_:
  - _conceal_aud_value_:
  - _captionType_:


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



