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
#include "ccdecoder_internal.h"
#include <stdlib.h>
#include <float.h>
#ifdef CCDECODER_DUMP
    #include <stdio.h>
#endif //CCDECODER_DUMP
//---------------------------------------------------------------------------

/****************************************************************************/
/*                                                                          */
/****************************************************************************/

typedef struct ccdecoder_internal_priv
{
    size_t                              handle_created_by;
    #ifdef CCDECODER_DUMP
        uint64_t                        debug_packets_count;                  
        uint64_t                        debug_bytes_count;                  
        uint64_t                        debug_files_count;                  
    #endif //CCDECODER_DUMP
} ccdecoder_internal_priv;

struct ccdecoder_handle* ccdecoder_internal_alloc  ()
{
    struct ccdecoder_handle* handle=(ccdecoder_handle*)malloc(sizeof(ccdecoder_handle));
    if (handle==NULL)
        return NULL;

    handle->user=NULL;
    handle->priv=NULL;
    handle->count=0;
    handle->transports=NULL;
    handle->pts=DBL_MAX;
    handle->dts=DBL_MAX;
    handle->dur=DBL_MAX;

    return handle;
}

size_t ccdecoder_internal_parse_begin   (ccdecoder_handle* handle, uint8_t* data, size_t size, size_t options, size_t handle_created_by)
{
    size_t transport_pos, caption_pos;
    ccdecoder_internal_priv* priv;
    #ifdef CCDECODER_DUMP
        char Debug_File[100];
        FILE* Debug_F;
        #ifdef WIN32
            #define DEBUG_DIR ""
        #else //WIN32
            #define DEBUG_DIR "/tmp/"
        #endif //WIN32
    #endif //CCDECODER_DUMP

    priv=(ccdecoder_internal_priv*)handle->priv;
   
    #ifdef CCDECODER_DUMP
    if (handle_created_by==priv->handle_created_by)
    {
        sprintf(Debug_File, DEBUG_DIR"ccdecoder.%p.raw", priv);
        if (priv->debug_packets_count==0)
        {
            remove(Debug_File);
            priv->debug_files_count++;
        }
        if (priv->debug_bytes_count>=priv->debug_files_count*100000000)
        {
            char Debug_File1[100+2];
            sprintf(Debug_File1, "%s.1", Debug_File);
            remove(Debug_File1);
            rename(Debug_File, Debug_File1);
            priv->debug_files_count++;
        }
        Debug_F=fopen(Debug_File, "ab");
        if (Debug_F)
        {
            fwrite(data, size, 1, Debug_F);
            fclose(Debug_F);
        }
        fprintf(stderr, "ccdecoder dump: %p, packet %i, parsing %i bytes, ", handle, (int)priv->debug_packets_count, (int)size);
        priv->debug_packets_count++;                  
        priv->debug_bytes_count+=size;
    }
    #endif //CCDECODER_DUMP

    if (handle->transports==NULL)
        ccdecoder_internal_bind(handle, handle, 0);

    if (handle_created_by==priv->handle_created_by)
        for (transport_pos=0; transport_pos<handle->count; transport_pos++)
            if (handle->transports[transport_pos])
            {
                for (caption_pos=0; caption_pos<handle->transports[transport_pos]->count; caption_pos++)
                    if (handle->transports[transport_pos]->captions[caption_pos])
                        handle->transports[transport_pos]->captions[caption_pos]->status=0;

                // Raw stream reset
                handle->transports[transport_pos]->raw_count=0;
            }

    return 0;
}

size_t ccdecoder_internal_parse_end (ccdecoder_handle* handle, size_t status, size_t handle_created_by)
{
    #ifdef CCDECODER_DUMP
    ccdecoder_internal_priv* priv;

    priv=(ccdecoder_internal_priv*)handle->priv;
   
    if (handle_created_by==priv->handle_created_by)
    {
        fprintf(stderr, "status=%x\n", (int)status);
    }
    #endif //CCDECODER_DUMP

    return 0;
}

void  ccdecoder_internal_bind  (ccdecoder_handle* handle, ccdecoder_handle* to_bind, size_t offset)
{
    ccdecoder_internal_priv* tobind_priv;
    ccdecoder_internal_priv* handle_priv;

    if (handle==NULL || to_bind==NULL)
        return;

    if (handle->transports==NULL)
    {
        handle->transports=(ccdecoder_transport**)calloc(2, sizeof(ccdecoder_transport*)); //Prepared to receive maximum 2 transport methods (e.g. SCTE 20 and ATSC/35)
        if (handle->transports==NULL)
            return; //ccdecoder_memoryerror;
    }

    tobind_priv=(ccdecoder_internal_priv*)to_bind->priv;
    handle_priv=(ccdecoder_internal_priv*)handle->priv;
    tobind_priv->handle_created_by=handle_priv->handle_created_by;

    to_bind->transports=handle->transports+offset;
    if (handle->count<=offset)
        handle->count=offset+1;
}

void  ccdecoder_internal_free  (ccdecoder_handle* handle, size_t handle_created_by)
{
    size_t transport_pos, pos;
    struct ccdecoder_internal_priv* priv;

    if (handle==NULL)
        return;

    priv=(ccdecoder_internal_priv*)handle->priv;

    if (priv->handle_created_by==handle_created_by)
    {
        for (transport_pos=0; transport_pos<handle->count; transport_pos++)
            if (handle->transports[transport_pos])
            {
                for (pos=0; pos<handle->transports[transport_pos]->count; pos++)
                    if (handle->transports[transport_pos]->captions[pos])
                    {
                        if (handle->transports[transport_pos]->captions[pos]->minimal)
                        {
                            size_t Line;

                            for (Line=0; handle->transports[transport_pos]->captions[pos]->minimal[Line]; Line++)
                                free(handle->transports[transport_pos]->captions[pos]->minimal[Line]);

                            free(handle->transports[transport_pos]->captions[pos]->minimal);
                        }
                
                        free (handle->transports[transport_pos]->captions[pos]);
                    }
                free(handle->transports[transport_pos]->captions);

                for (pos=0; pos<handle->transports[transport_pos]->raw_count; pos++)
                    if (handle->transports[transport_pos]->raw[pos])
                    {
                        free(handle->transports[transport_pos]->raw[pos]->data);
                        free(handle->transports[transport_pos]->raw[pos]);
                    }
                free(handle->transports[transport_pos]->raw);

                for (pos=0; pos<handle->transports[transport_pos]->encoded_count; pos++)
                    if (handle->transports[transport_pos]->encoded[pos])
                    {
                        free(handle->transports[transport_pos]->encoded[pos]->data);
                        free(handle->transports[transport_pos]->encoded[pos]);
                    }
                free(handle->transports[transport_pos]->encoded);

                if (handle->transports[transport_pos]->dump)
                {
                    free(handle->transports[transport_pos]->dump->data);
                    free(handle->transports[transport_pos]->dump);
                }

                free(handle->transports[transport_pos]);
            }

        free(handle->transports);
    }

    free(handle);
}

ccdecoder_caption** ccdecoder_captions_init  ()
{
    return (ccdecoder_caption**)calloc(ccdecoder_order_max, sizeof(ccdecoder_caption*));
}

ccdecoder_caption* ccdecoder_caption_init   ()
{
    ccdecoder_caption* caption=(ccdecoder_caption*)calloc(1, sizeof(ccdecoder_caption));

    caption->user=0;
    caption->status=0;
    caption->minimal=NULL;
    caption->full=NULL;
    caption->xds=NULL;

    return caption;
}

/****************************************************************************/
/* Bit stream helper                                                        */
/****************************************************************************/

//---------------------------------------------------------------------------
#ifndef min
    #define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

//---------------------------------------------------------------------------
// Alloc
bitstream* bitstream_alloc (uint8_t* buffer_, size_t size_)
{
    bitstream* BS=(bitstream*)malloc(sizeof (bitstream));
    
    BS->buffer=buffer_;
    BS->buffer_size=BS->buffer_size_Init=size_*8; //size is in bits
    BS->bufferunderrun=BS->buffer_size?0:1;

    return BS;
}

//---------------------------------------------------------------------------
// Get
uint32_t bitstream_get (bitstream* BS, size_t HowMany)
{
    size_t ToReturn;
    size_t NewBits;
    static const uint32_t Mask[33]={
        0x00000000,
        0x00000001, 0x00000003, 0x00000007, 0x0000000f,
        0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
        0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
        0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
        0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
        0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
        0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
        0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff,
    };

    if (HowMany==0 || HowMany>32)
        return 0;

    if (HowMany<=(BS->buffer_size%8))
    {
        BS->buffer_size-=HowMany;
        return (BS->lastbyte>>(BS->buffer_size%8))&Mask[HowMany];
    }

    if (HowMany>BS->buffer_size)
    {
        BS->buffer_size=0;
        BS->bufferunderrun=1;
        return 0;
    }

    NewBits=HowMany-(BS->buffer_size%8);
    if (NewBits==32)
        ToReturn=0;
    else
        ToReturn=BS->lastbyte<<NewBits;
    switch ((NewBits-1)>>8)
    {
        case 3 :    NewBits-=8;
                    ToReturn|=*BS->buffer<<NewBits;
                    BS->buffer++;
        case 2 :    NewBits-=8;
                    ToReturn|=*BS->buffer<<NewBits;
                    BS->buffer++;
        case 1 :    NewBits-=8;
                    ToReturn|=*BS->buffer<<NewBits;
                    BS->buffer++;
        default:
                    ;
    }
    BS->lastbyte=*BS->buffer;
    BS->buffer++;
    BS->buffer_size-=HowMany;
    ToReturn|=(BS->lastbyte>>(BS->buffer_size%8))&Mask[NewBits];
    return (uint32_t)(ToReturn&Mask[HowMany]);
};

//---------------------------------------------------------------------------
// Get
void bitstream_set (bitstream* BS, size_t HowMany, uint32_t Value)
{
    static const uint32_t Mask[33]={
        0x00000000,
        0x00000001, 0x00000003, 0x00000007, 0x0000000f,
        0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
        0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
        0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
        0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
        0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
        0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
        0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff,
    };

    if (HowMany==0 || HowMany>32 || Value>Mask[HowMany])
        return;

    if (HowMany>BS->buffer_size)
    {
        BS->buffer_size=0;
        BS->bufferunderrun=1;
        return;
    }

    do
    {
        size_t lastbyte_size=BS->buffer_size%8;

        if (lastbyte_size==0)
        {
            BS->lastbyte=*BS->buffer;
            BS->buffer++;
            lastbyte_size=8;
        }

        if (HowMany<=lastbyte_size)
        {
            uint8_t Mask_Local=Mask[HowMany]<<(lastbyte_size-HowMany);

            BS->lastbyte&=~Mask_Local;
            BS->lastbyte|=Value<<(lastbyte_size-HowMany);
            *(BS->buffer-1)=BS->lastbyte;
            BS->buffer_size-=HowMany;
            return;
        }
        else
        {
            // Remaining byte
            {
                uint8_t Mask_Local=Mask[lastbyte_size];

                BS->lastbyte&=~Mask_Local;
                BS->lastbyte|=Value>>(HowMany-lastbyte_size);
                *(BS->buffer-1)=BS->lastbyte;

                HowMany-=lastbyte_size;
                BS->buffer_size-=lastbyte_size;
                Value&=Mask[HowMany];
            }
        }
    }
    while (HowMany);
};

//---------------------------------------------------------------------------
// Unsigned Exp Colomb
uint32_t bitstream_get_ue (bitstream* BS)
{
    int LeadingZeroBits=0;
    while(bitstream_remain(BS)>0 && bitstream_get(BS, 1)==0)
    {
        LeadingZeroBits++;
        if (LeadingZeroBits>31)
            return 0; // Problem
    }

    if (LeadingZeroBits>31)
        return 0; // Problem
    return (1<<LeadingZeroBits)-1+bitstream_get(BS, LeadingZeroBits);
}

//---------------------------------------------------------------------------
// Signed Exp Colomb
int32_t bitstream_get_se (bitstream* BS)
{
    int LeadingZeroBits=0;
    uint32_t Info;
    int32_t Signed=0;
    while(bitstream_remain(BS)>0 && bitstream_get(BS, 1)==0)
    {
        LeadingZeroBits++;
        if (LeadingZeroBits>31)
            return 0; // Problem
    }

    if (LeadingZeroBits>31)
        return 0; // Problem
    Info=(1<<LeadingZeroBits)-1+bitstream_get(BS, LeadingZeroBits);
    if ((Info%2)==0)
        Signed=1;
    return (int32_t)((Signed?-1:1)*(((Info%2)?1:0)+(Info>>1)));
}

//---------------------------------------------------------------------------
// How many bits remain
uint32_t bitstream_remain (bitstream* BS)
{
    return (uint32_t)BS->buffer_size;
}

//---------------------------------------------------------------------------
// Free
void bitstream_free (bitstream* BS)
{
    free(BS);
    BS=NULL;
}

