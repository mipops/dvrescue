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

//---------------------------------------------------------------------------
#ifndef ccdecoder_internalH
#define ccdecoder_internalH
//---------------------------------------------------------------------------

#include "ccdecoder_common.h"

#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus

/****************************************************************************/
/* Functions                                                                */
/****************************************************************************/

ccdecoder_handle*   ccdecoder_internal_alloc        ();
size_t              ccdecoder_internal_parse_begin  (ccdecoder_handle* handle, uint8_t* data, size_t size, size_t options, size_t handle_created_by);
size_t              ccdecoder_internal_parse_end    (ccdecoder_handle* handle, size_t status, size_t handle_created_by);
void                ccdecoder_internal_bind         (ccdecoder_handle* handle, ccdecoder_handle* to_bind, size_t offset);
void                ccdecoder_internal_free         (ccdecoder_handle* handle, size_t handle_created_by);
ccdecoder_caption** ccdecoder_captions_init         ();
ccdecoder_caption*  ccdecoder_caption_init          ();

/****************************************************************************/
/* Bit stream helper                                                        */
/****************************************************************************/

//---------------------------------------------------------------------------
// Structure
typedef struct bitstream
{
    uint8_t*        buffer;
    size_t          buffer_size;
    size_t          buffer_size_Init;
    uint8_t         lastbyte;
    int8_t          bufferunderrun; // bool
} bitstream;

//---------------------------------------------------------------------------
// Functions
bitstream*  bitstream_alloc     (uint8_t* buffer, size_t size);
uint32_t    bitstream_get       (bitstream* BS, size_t HowMany);
void        bitstream_set       (bitstream* BS, size_t HowMany, uint32_t Value);
uint32_t    bitstream_get_ue    (bitstream* BS);
int32_t     bitstream_get_se    (bitstream* BS);
uint32_t    bitstream_remain    (bitstream* BS);
void        bitstream_free      (bitstream* BS);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* ccdecoderH */
