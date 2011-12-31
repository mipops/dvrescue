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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ccdecoder_line21.h"
#include "ccdecoder_line21field.h"
#include "ccdecoder_internal.h"

typedef struct ccdecoder_line21_priv
{
    size_t                              handle_created_by;
    #ifdef CCDECODER_DUMP
        uint64_t                        debug_packets_count;                  
        uint64_t                        debug_bytes_count;                  
        uint64_t                        debug_files_count;                  
    #endif //CCDECODER_DUMP

    ccdecoder_handle*                   parsers[2];
} ccdecoder_line21_priv;

/****************************************************************************/
/* Public functions                                                         */
/****************************************************************************/

//---------------------------------------------------------------------------
// Alloc
ccdecoder_handle* ccdecoder_line21_alloc ()
{
    ccdecoder_handle*                   handle;
    ccdecoder_line21_priv*              priv;

    handle=ccdecoder_internal_alloc();
    if (handle==NULL)
        return NULL;

    handle->priv=priv=(ccdecoder_line21_priv*)malloc(sizeof(ccdecoder_line21_priv));
    priv->handle_created_by=0x24;

    priv->parsers[0]=NULL;
    priv->parsers[1]=NULL;

    return handle;
}

size_t ccdecoder_line21_parse (ccdecoder_handle* handle, uint8_t* data, size_t size, size_t options, double pts, double dts, double dur)
{
    ccdecoder_line21_priv* priv;
    size_t  status=0;
    size_t  offset;
    int     has1=options&ccdecoder_fromfield1;
    int     has2=options&ccdecoder_fromfield2;

    if (handle==NULL)
        return ccdecoder_nochange;
    if (ccdecoder_internal_parse_begin(handle, data, size, options, 0x24))
        return ccdecoder_memoryerror;
    if (handle->transports[0]==NULL)
    {
        handle->count=1;
        handle->transports[0]=(ccdecoder_transport*)calloc(1, sizeof(ccdecoder_transport));
        handle->transports[0]->user=NULL;
        handle->transports[0]->format=ccdecoder_dtvcc;
        handle->transports[0]->count=0;
        handle->transports[0]->captions=NULL;
        handle->transports[0]->raw_count=0;
        handle->transports[0]->raw=NULL;
        handle->transports[0]->info_fieldrate=0;
        handle->transports[0]->encoded_count=0;
        handle->transports[0]->encoded=NULL;
        handle->transports[0]->dump=NULL;
    }

    priv=(ccdecoder_line21_priv*)handle->priv;

    if (!has1 && !has2)
    {
        has1=ccdecoder_fromfield1;
        has2=ccdecoder_fromfield2;
    }
    if ((!has1 &&  has2 && size!=2)
     || ( has1 && !has2 && size!=2)
     || ( has1 &&  has2 && size!=4))
        return 0;

    // Raw stream export
    if (handle->transports[0]->raw==NULL)
        handle->transports[0]->raw=(ccdecoder_rawdata**)calloc(2, sizeof(ccdecoder_rawdata*));
    handle->transports[0]->raw_count=size/2;

    for (offset=0; offset<size; offset+=2)
    {
        uint8_t ccdecoder_type;
        size_t raw_offset=offset;
        if (has1 && offset/2==0)
            ccdecoder_type=0; //field 1
        else
            ccdecoder_type=1; //field 2
        
        // Raw stream export
        if (offset==0 && handle->transports[0]->raw[0] && handle->transports[0]->raw[0]->size==3 && (handle->transports[0]->raw[1]==NULL || handle->transports[0]->raw[1]->size==0)) // In case CC are sent in 2 separate calls
        {
            raw_offset+=2;
            handle->transports[0]->raw_count=raw_offset/2+1;
        }
        if (handle->transports[0]->raw[raw_offset/2]==NULL)
        {
            handle->transports[0]->raw[raw_offset/2]=(ccdecoder_rawdata*)calloc(1, sizeof(ccdecoder_rawdata));
            handle->transports[0]->raw[raw_offset/2]->data=(unsigned char*)calloc(3, 1);
        }
        handle->transports[0]->raw[raw_offset/2]->data[0]=ccdecoder_type?0xFD:0xFC; // 11111 + valid bit + field
        memcpy(handle->transports[0]->raw[raw_offset/2]->data+1, data+offset, 2);
        handle->transports[0]->raw[raw_offset/2]->size=3;

        // Decode
        if (priv->parsers[ccdecoder_type]==NULL)
        {
            priv->parsers[ccdecoder_type]=ccdecoder_line21field_alloc(ccdecoder_type+1);
            ccdecoder_internal_bind(handle, priv->parsers[ccdecoder_type], 0);
        }

        // Status
        status|=ccdecoder_line21field_parse(priv->parsers[ccdecoder_type], data+offset, 2, options, 0, 0, 0);
    }

    ccdecoder_internal_parse_end(handle, status, 0x24);
    return status;
}

void  ccdecoder_line21_free  (ccdecoder_handle* handle)
{
    ccdecoder_line21_priv* priv;

    if (handle==NULL)
        return;

    priv=(ccdecoder_line21_priv*)handle->priv;
    if (priv==NULL)
        return;

    ccdecoder_line21field_free(priv->parsers[0]);
    ccdecoder_line21field_free(priv->parsers[1]);

    ccdecoder_internal_free(handle, 0x24);

    free(priv);
}
