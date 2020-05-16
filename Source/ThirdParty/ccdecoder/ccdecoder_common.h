/*
 * ccdecoder - A standalone Closed Captions (CC) decoder
 * Copyright (C) 2012-2012 MediaArea.net SARL, Info@MediaArea.net
 * Copyright (C) 2012-2012 RGB Networks
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *          
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 ***************************************************************************/

#ifndef ccdecoder_commonH
#define ccdecoder_commonH

// For debugging
//#define CCDECODER_DUMP

#include <wchar.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus

/**
 * @brief ccdecoder
 * @version 1.1
 * \mainpage ccdecoder main page
 * This library is intended as a compilation of APIs (depending of how you want to feed the decoder) for caption decoding and display.
 * 
 * You can access the decoder with:
 * - coded stream: an MPEG-2 Video stream with SCTE 20 or ATSC A/52 (SCTE 21) user data, an H264/AVC stream with SCTE 128 user data
 * - transport layer: a DTVCC transport stream, an SCTE 20 stream, ancillary data (VANC)
 * - caption layer: a line 21 stream, a DTVCC caption stream, a CDP stream
 *
 * Then you can use the decoded caption:
 * - directly display it on screen
 * - convert it to Subrip format
 * - create your own transform function
 *
 * It is also possible to transcode to:
 * - DTVCC transport stream (with A/52 or SCTE 128 header)
 * - SCTE 20
 *
 * Examples about how to use the decoder API, with frames from FFmpeg (or libav) or from VANC, are also provided (ccdecoder_ffmpeg.c and ccdecoder_fromfile.c)
 *
 * \section main Main interface
 * This interface accepts MPEG-2 Video or H264/AVC frames as input, without any need to specifiy the format (it is automaticly detected)
 * - Create an handle with ccdecoder_alloc()
 * - Parse video frames with ccdecoder_parse(), as long as you need
 * - Delete the handle with ccdecoder_free()
 * \section mpeg2 MPEG-2 Video interface
 * This interface accepts MPEG-2 Video frames as input
 * - Create an handle with ccdecoder_mpegv_alloc()
 * - Parse video frames with ccdecoder_mpegv_parse(), as long as you need
 * - Delete the handle with ccdecoder_mpegv_free()
 * \section avc H264/AVC interface
 * This interface accepts H264/AVC frames as input
 * - Create an handle with ccdecoder_avc_alloc()
 * - Parse video frames with ccdecoder_avc_parse(), as long as you need
 * - Delete the handle with ccdecoder_avc_free()
 * \section dtvcc_transport DTVCC Transport interface
 * This interface accepts DTVCC Transport blocks (in presentation order) as input
 * - Create an handle with ccdecoder_dtvcc_transport_alloc()
 * - Parse video frames with ccdecoder_dtvcc_transport_parse(), as long as you need
 * - Delete the handle with ccdecoder_dtvcc_transport_free()
 * \section scte20 SCTE 20 interface
 * This interface accepts SCTE 20 blocks (in presentation order) (without user_data_type_code byte), as input
 * - Create an handle with ccdecoder_scte20_alloc()
 * - Parse video frames with ccdecoder_scte20_parse(), as long as you need
 * - Delete the handle with ccdecoder_scte20_free()
 * \section cdp CDP interface
 * This interface accepts CDP blocks (in presentation order) as input
 * - Create an handle with ccdecoder_cdp_transport_alloc()
 * - Parse video frames with ccdecoder_cdp_transport_parse(), as long as you need
 * - Delete the handle with ccdecoder_cdp_transport_free()
 * \section smpte291 SMPTE S291M interface
 * This interface accepts SMPTE S291M blocks (in presentation order), including SMPTE 334M, as input
 * - Create an handle with ccdecoder_smpte291_transport_alloc()
 * - Parse video frames with ccdecoder_smpte291_transport_parse(), as long as you need
 * - Delete the handle with ccdecoder_smpte291_transport_free()
 * Note: byte stream can bit 8 or 10 bits, big or little endian, without or without header, with or without footer
 * \section line21 Line 21 interface
 * This interface accepts Line 21 (CEA-608) packets (in presentation order) as input
 * - Create an handle with ccdecoder_line21_alloc()
 * - Parse video frames with ccdecoder_line21_parse(), as long as you need
 * - Delete the handle with ccdecoder_line21field_free()
 * \section dtvcc_caption DTVCC Caption interface
 * This interface accepts DTVCC Caption packets (in presentation order) as input
 * - Create an handle with ccdecoder_dtvcc_caption_alloc()
 * - Parse video frames with ccdecoder_dtvcc_caption_parse(), as long as you need
 * - Delete the handle with ccdecoder_dtvcc_caption_free()
 * \section onscreen "On Screen" output example
 * This interface accepts an ccdecoder handle (in presentation order) as input
 * - Output the current captions with ccdecoder_onscreen()
 * \section subrip Subrip output example
 * This interface accepts an ccdecoder handle (in presentation order) as input
 * - Create an handle with ccdecoder_subrip_alloc()
 * - Parse the ccdecoder handle with ccdecoder_subrip_parse() when the decoder handle status has changed (status has the flag "ccdecoder_haschanged" set), result is a subrip-compatible text block
 * - Delete the handle with ccdecoder_subrip_free()
 * \section dtvcc_transport_export DTVCC Transport output example
 * This interface accepts an ccdecoder handle (in presentation order) as input
 * - Parse the ccdecoder handle with ccdecoder_dtvcc_transport_export_parse()
 * \section export Export output example
 * This interface accepts an ccdecoder transport (in presentation order) as input, output is SCTE 20/21/128. It accepts asynchronized export (decode in one thread, encode in another thread). If asynchronized export is in use, it is possible to adapt the frame rate (from 30i/30p/60p to 30i/30p/60p).
 * - Create an handle with ccdecoder_export_alloc()
 * - Parse the ccdecoder handle with ccdecoder_export_parse()
 * - Delete the handle with ccdecoder_export_free()
 */

/****************************************************************************/
/* Enums and structures                                                     */
/****************************************************************************/

/**
 * To be used as the first indice of ccdecoder_minimal_character***\n
 * Summary of specifciations:\n
 * Line21 (CEA-608): has 2 streams, one per field, even if it is a frame\n
 * for each stream, there are 2 Caption (CC) and 2 Text (T) sub-streams\n
 * Field 1: CC1/CC2/T1/T2\n
 * Field 2: CC3/CC4/T3/T4\n
 * DTVCC (CEA-708): has 63 Caption sub-streams (services)\n
 * No service 0, start from 1 and is up to 63
 */
enum ccdecoder_order
{
    ccdecoder_line21_CC1,                                       ///< Line 21 (CEA-608) 1st field 1st caption (CC1)
    ccdecoder_line21_CC2,                                       ///< Line 21 (CEA-608) 1st field 2nd caption (CC2)
    ccdecoder_line21_T1,                                        ///< Line 21 (CEA-608) 1st field 1st text (T1)
    ccdecoder_line21_T2,                                        ///< Line 21 (CEA-608) 1st field 2nd text (T2)
    ccdecoder_line21_CC3,                                       ///< Line 21 (CEA-608) 2nd field 1st caption (CC3)
    ccdecoder_line21_CC4,                                       ///< Line 21 (CEA-608) 2nd field 2nd caption (CC4)
    ccdecoder_line21_T3,                                        ///< Line 21 (CEA-608) 2nd field 1st text (T3)
    ccdecoder_line21_T4,                                        ///< Line 21 (CEA-608) 2nd field 2nd text (T4)
    ccdecoder_reserved_8,                                       ///< Reserved, is not used
    ccdecoder_dtvcc_1,                                          ///< DTVCC service 1
    ccdecoder_dtvcc_2,                                          ///< DTVCC service 2
    ccdecoder_dtvcc_3,                                          ///< DTVCC service 3
    ccdecoder_dtvcc_4,                                          ///< DTVCC service 4
    ccdecoder_dtvcc_5,                                          ///< DTVCC service 5
    ccdecoder_dtvcc_6,                                          ///< DTVCC service 6
    ccdecoder_dtvcc_7,
    ccdecoder_dtvcc_8,
    ccdecoder_dtvcc_9,
    ccdecoder_dtvcc_10,
    ccdecoder_dtvcc_11,
    ccdecoder_dtvcc_12,
    ccdecoder_dtvcc_13,
    ccdecoder_dtvcc_14,
    ccdecoder_dtvcc_15,
    ccdecoder_dtvcc_16,
    ccdecoder_dtvcc_17,
    ccdecoder_dtvcc_18,
    ccdecoder_dtvcc_19,
    ccdecoder_dtvcc_20,
    ccdecoder_dtvcc_21,
    ccdecoder_dtvcc_22,
    ccdecoder_dtvcc_23,
    ccdecoder_dtvcc_24,
    ccdecoder_dtvcc_25,
    ccdecoder_dtvcc_26,
    ccdecoder_dtvcc_27,
    ccdecoder_dtvcc_28,
    ccdecoder_dtvcc_29,
    ccdecoder_dtvcc_30,
    ccdecoder_dtvcc_31,
    ccdecoder_dtvcc_32,
    ccdecoder_dtvcc_33,
    ccdecoder_dtvcc_34,
    ccdecoder_dtvcc_35,
    ccdecoder_dtvcc_36,
    ccdecoder_dtvcc_37,
    ccdecoder_dtvcc_38,
    ccdecoder_dtvcc_39,
    ccdecoder_dtvcc_40,
    ccdecoder_dtvcc_41,
    ccdecoder_dtvcc_42,
    ccdecoder_dtvcc_43,
    ccdecoder_dtvcc_44,
    ccdecoder_dtvcc_45,
    ccdecoder_dtvcc_46,
    ccdecoder_dtvcc_47,
    ccdecoder_dtvcc_48,
    ccdecoder_dtvcc_49,
    ccdecoder_dtvcc_50,
    ccdecoder_dtvcc_51,
    ccdecoder_dtvcc_52,
    ccdecoder_dtvcc_53,
    ccdecoder_dtvcc_54,
    ccdecoder_dtvcc_55,
    ccdecoder_dtvcc_56,
    ccdecoder_dtvcc_57,
    ccdecoder_dtvcc_58,
    ccdecoder_dtvcc_59,
    ccdecoder_dtvcc_60,
    ccdecoder_dtvcc_61,
    ccdecoder_dtvcc_62,
    ccdecoder_dtvcc_63,
    ccdecoder_order_max,
};

/**
 * Options of the caption decoder
 */
typedef enum ccdecoder_options
{
    ccdecoder_nooptions             =0x00000000,    ///< No options

    ccdecoder_oneframe              =0x00000001,    ///< The input contains only one frame (use it to improve speed if you already split the input per frame) @note bitwise
    ccdecoder_unsynched             =0x00000002,    ///< The input was unsynched, the parser must resynchronized @note bitwise
    ccdecoder_synched               =0x00000004,    ///< The input was synched from the underlying layer @note bitwise

    ccdecoder_is16bit               =0x00000010,    ///< The input has 16-bit per pixel @note bitwise
    ccdecoder_islittleendian        =0x00000020,    ///< The input is in Little Endian @note bitwise
    ccdecoder_hasheader             =0x00000040,    ///< The input contains an optional header (e.g. synchro) @note bitwise
    ccdecoder_hasfooter             =0x00000080,    ///< The input contains an optional footer (e.g. CRC) @note bitwise

    ccdecoder_fromfield1            =0x00000100,    ///< The input contains data from field 1 @note bitwise
    ccdecoder_fromfield2            =0x00000200,    ///< The input contains data from field 2 @note bitwise

    ccdecoder_encodeddump           =0x00001000,    ///< The parser is requested to dump the data in the encoded[] part of the transport pointer
    ccdecoder_flush                 =0x00002000,    ///< The parser is requested to flush the caption stream
} ccdecoder_options;

/**
 * Status of the caption decoder
 */
typedef enum ccdecoder_status
{
    ccdecoder_nochange              =0x00000000,    ///< Nothing has changed @note bitwise
    ccdecoder_unsupportedoption     =0x00000001,    ///< At least one of the options, or a mix of the options, is not supported @note bitwise
    ccdecoder_parsingerror          =0x00000002,    ///< There was an parsing error (due to the input) during the parsing @note bitwise
    ccdecoder_memoryerror           =0x00000004,    ///< There was a memory error during the parsing @note bitwise
    ccdecoder_issynched             =0x00000010,    ///< The stream is synched (header is found, parsing is normal)
    ccdecoder_haschanged            =0x00000020,    ///< The output has changed since the last parsing @note bitwise
    ccdecoder_isflushed             =0x00000040,    ///< The output is flushed, no more data in buffer @note bitwise
} ccdecoder_status;

/**
 * Cosmetic attributes for a ccdecoder_minimal_character
 */
typedef enum ccdecoder_attributes
{
    ccdecoder_noattribute           =0x00,          ///< No attribute
    ccdecoder_white                 =0x00,          ///< Character color is white @note must be filtered with "&0x7" (3 lower bits)
    ccdecoder_green                 =0x01,          ///< Character color is green @note must be filtered with "&0x7" (3 lower bits)
    ccdecoder_blue                  =0x02,          ///< Character color is blue @note must be filtered with "&0x7" (3 lower bits)
    ccdecoder_cyan                  =0x03,          ///< Character color is cyan @note must be filtered with "&0x7" (3 lower bits)
    ccdecoder_red                   =0x04,          ///< Character color is red @note must be filtered with "&0x7" (3 lower bits)
    ccdecoder_yellow                =0x05,          ///< Character color is yellow @note must be filtered with "&0x7" (3 lower bits)
    ccdecoder_magenta               =0x06,          ///< Character color is magenta @note must be filtered with "&0x7" (3 lower bits)
    ccdecoder_underline             =0x10,          ///< Character is underlined @note bitwise
    ccdecoder_italic                =0x20,          ///< Character is in italic @note bitwise
} ccdecoder_attributes;

/**
 * Format of the transport stream
 */
typedef enum ccdecoder_format
{
    ccdecoder_noformat              =0x00,          ///< No format
    ccdecoder_dtvcc                 =0x01,          ///< DTVCC Transport (via SCTE 21 or SCTE 128)
    ccdecoder_scte20                =0x02,          ///< SCTE 20
    ccdecoder_cdp                   =0x03,          ///< CDP
    ccdecoder_smpte291_cdp          =0x04,          ///< SMPTE 291, CDP (via SMPTE 334)
    ccdecoder_smpte291_line21       =0x05,          ///< SMPTE 291, Line 21 (via SMPTE 334)
} ccdecoder_format;

/**
 * A ccdecoder_minimal_character
 */
typedef struct ccdecoder_minimal_character
{
    wchar_t                         value;          ///< Raw character, in Unicode format
    ccdecoder_attributes            attributes;     ///< Optional cosmetic attributes
} ccdecoder_minimal_character;

/**
 * A caption stream
 */
typedef struct ccdecoder_caption
{
    void*                           user;           ///< User usage
    size_t                          status;         ///< status of sub-stream, per caption sub-stream 
    ccdecoder_minimal_character**   minimal;        ///< Array of characters, minimal output as a 15 lines x 32/42 columns (4:3 or 16:9), line is finishing with an extra character with value=L'\0'. Per caption sub-stream
    void*                           full;           ///< Not implemented
    void*                           xds;            ///< Not implemented
} ccdecoder_caption;

/**
 * A raw stream (CEA-608, DTVCC)
 */
typedef struct ccdecoder_rawdata
{
    size_t                          size;           ///< Size of data
    unsigned char*                  data;           ///< raw data
} ccdecoder_rawdata;

/**
 * A caption transport stream
 */
typedef struct ccdecoder_transport
{
    void*                           user;           ///< User usage
    size_t                          format;         ///< 
    size_t                          count;          ///< Count of maximal active sub-streams
    ccdecoder_caption**             captions;       ///< 
    size_t                          raw_count;      ///< Count of raw blocks
    ccdecoder_rawdata**             raw;            ///< List of pointers to a block with raw data. 1st block is CEA-608 field 1, 2nd block is CEA-608 field 2, other blocks are CEA-708 (Note: CEA-708 not yet implemented)
    size_t                          info_fieldrate; ///< Maximum expected fields/frames per second, rounded
    size_t                          encoded_count;  ///< Count of raw blocks, in encoded order
    ccdecoder_rawdata**             encoded;        ///< List of pointers to a block with raw data, in encoded order. 1st block is CEA-608 field 1, 2nd block is CEA-608 field 2, other blocks are CEA-708 (Note: CEA-708 not yet implemented)
    ccdecoder_rawdata*              dump;           ///< Dump of the raw data, including transport layer header, excluding video stream header
} ccdecoder_transport;

/**
 * Main ccdecoder handle
 */
typedef struct ccdecoder_handle
{
    void*                           user;           ///< User usage
    void*                           priv;           ///< priv (internal usage), do not use it
    size_t                          count;          ///< Count of maximal active transports
    ccdecoder_transport**           transports;     ///< 
    double                          pts;            ///< Presentation time stamp (in seconds)
    double                          dts;            ///< Decoding time stamp (in seconds)
    double                          dur;            ///< Duration (in seconds)
} ccdecoder_handle;

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* ccdecoder_commonH */
