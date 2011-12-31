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
#include "ccdecoder_line21field.h"
#include "ccdecoder_internal.h"

/****************************************************************************/
/* Constants                                                                */
/****************************************************************************/

/* Screen size, specified by the standard */
const size_t line21_Minimal_Rows=15;
const size_t line21_Minimal_Columns=32;

//---------------------------------------------------------------------------
// CAE-608-E section F.1.1.5
static const uint8_t Eia608_PAC_Row[]=
{
    10,
    0,  //or 1
    2,  //or 3
    11, //or 12
    13, //or 14
    4,  //or 5
    6,  //or 7
    8   //or 9
};

//---------------------------------------------------------------------------
// Private
typedef struct ccdecoder_line21_service
{
    uint8_t     InBack; //The back buffer is written (bool)
    size_t      x;
    size_t      y;
    ccdecoder_attributes Attribute_Current;
    size_t      RollUpLines;
    ccdecoder_minimal_character** CC_NonDisplayed;
    uint8_t     Synched;
    size_t      NoChangeDuration;
} ccdecoder_line21_service;
typedef struct ccdecoder_line21_xds
{
    uint8_t     data[36]; // 2 Start + 32 Data + 2 End
    size_t      size;
} ccdecoder_line21_xds;
typedef struct ccdecoder_line21field_priv
{
    size_t                              handle_created_by;
    #ifdef CCDECODER_DUMP
        uint64_t                        debug_packets_count;                  
        uint64_t                        debug_bytes_count;                  
        uint64_t                        debug_files_count;                  
    #endif //CCDECODER_DUMP

    ccdecoder_handle*   CC;
    size_t              CC_Offset;
    uint8_t     TextMode; //CC or T (bool)
    uint8_t     dataChannelMode; //if 1, CC2/CC4/T2/T4 (bool)
    uint8_t     ccdecoder_data_1_Old;
    uint8_t     ccdecoder_data_2_Old;
    uint8_t     FieldNumber;
    ccdecoder_line21_service* Service[4];
    ccdecoder_line21_xds*     XDS[8];
    size_t                    XDS_Level;
} ccdecoder_line21field_priv;

ccdecoder_minimal_character* ccdecoder_line21_alloc_display_line_alloc()
{
    ccdecoder_minimal_character* Display;
    size_t Column;

    Display=(ccdecoder_minimal_character*)calloc(line21_Minimal_Columns+1, sizeof(ccdecoder_minimal_character));
    for (Column=0; Column<line21_Minimal_Columns; Column++)
    {
        Display[Column].value=L' ';
        Display[Column].attributes=ccdecoder_noattribute;
    }
    Display[line21_Minimal_Columns].value=L'\0';
    Display[line21_Minimal_Columns].attributes=ccdecoder_noattribute;

    return Display;
}

void ccdecoder_line21_alloc_display_line_free(ccdecoder_minimal_character* Display)
{
    free(Display);
}

ccdecoder_minimal_character** ccdecoder_line21_alloc_display_alloc()
{
    ccdecoder_minimal_character** Display;
    size_t Line;

    Display=(ccdecoder_minimal_character**)calloc(line21_Minimal_Rows+1, sizeof(ccdecoder_minimal_character*));
    for (Line=0; Line<line21_Minimal_Rows; Line++)
        Display[Line]=ccdecoder_line21_alloc_display_line_alloc();
    Display[Line]=NULL;

    return Display;
}

void ccdecoder_line21_alloc_display_free(ccdecoder_minimal_character** Display)
{
    size_t Line;

    if (Display==NULL)
        return;

    for (Line=0; Line<line21_Minimal_Rows; Line++)
        ccdecoder_line21_alloc_display_line_free(Display[Line]);

    free(Display);
}

ccdecoder_line21_service* ccdecoder_line21_service_alloc (uint8_t IsText)
{
    ccdecoder_line21_service* Service=(ccdecoder_line21_service*)malloc(sizeof(ccdecoder_line21_service));
    if (Service==NULL)
        return NULL;

    Service->InBack=0;
    Service->x=0;
    Service->y=line21_Minimal_Rows-1;
    Service->Attribute_Current=ccdecoder_noattribute;
    Service->RollUpLines=0;
    Service->CC_NonDisplayed=IsText?NULL:ccdecoder_line21_alloc_display_alloc();
    Service->Synched=0;
    Service->NoChangeDuration=0;

    return Service;
}

void ccdecoder_line21_service_free (ccdecoder_line21_service* Service)
{
    if (Service==NULL)
        return;

    ccdecoder_line21_alloc_display_free(Service->CC_NonDisplayed);

    free (Service);
}

ccdecoder_handle* ccdecoder_line21field_alloc (uint8_t field)
{
    ccdecoder_handle*                   handle;
    ccdecoder_line21field_priv*         priv;
    uint8_t                             pos;

    if (field!=1 && field!=2)
        return NULL;

    handle=ccdecoder_internal_alloc();
    if (handle==NULL)
        return NULL;

    handle->priv=priv=(ccdecoder_line21field_priv*)malloc(sizeof(ccdecoder_line21field_priv));
    priv->handle_created_by=0x30;

    priv->CC=handle;
    priv->CC_Offset=(field-1)*4;
    priv->TextMode=0;
    priv->dataChannelMode=0;
    priv->ccdecoder_data_1_Old=0x00;
    priv->ccdecoder_data_2_Old=0x00;
    priv->FieldNumber=field-1;
    for (pos=0; pos<4; pos++)
        priv->Service[pos]=ccdecoder_line21_service_alloc(pos>>1);
    for (pos=0; pos<8; pos++)
        priv->XDS[pos]=(ccdecoder_line21_xds*)calloc(8, sizeof(ccdecoder_line21_xds));
    priv->XDS_Level=(size_t)-1;

    if (priv->Service[0]==NULL || priv->Service[1]==NULL || priv->Service[2]==NULL || priv->Service[3]==NULL)
    {
        ccdecoder_line21_service_free(priv->Service[0]);
        ccdecoder_line21_service_free(priv->Service[1]);
        ccdecoder_line21_service_free(priv->Service[2]);
        ccdecoder_line21_service_free(priv->Service[3]);
        return NULL; /* Problem */
    }

    return handle;
}

//---------------------------------------------------------------------------
void ccdecoder_line21_haschanged (ccdecoder_line21field_priv* priv)
{
    size_t pos=priv->TextMode*2+priv->dataChannelMode;

    // Filling
    if (priv->CC_Offset+pos>=priv->CC->transports[0]->count || priv->CC->transports[0]->captions[priv->CC_Offset+pos]==NULL)
        return; // Problem
    priv->CC->transports[0]->captions[priv->CC_Offset+pos]->status|=ccdecoder_haschanged;
}

//---------------------------------------------------------------------------
size_t ccdecoder_line21_returnvalue (ccdecoder_line21field_priv* priv)
{
    size_t status=0;
    uint8_t service_number;
    
    if (priv->CC->transports[0]->captions==NULL)
        return 0;

    for (service_number=0; service_number<4; service_number++)
        if (priv->CC->transports[0]->captions[priv->CC_Offset+service_number])
            status|=priv->CC->transports[0]->captions[priv->CC_Offset+service_number]->status;

    ccdecoder_internal_parse_end(priv->CC, status, 0x30);
    return status;
}

//---------------------------------------------------------------------------
void ccdecoder_line21_character_fill (ccdecoder_line21field_priv* priv, wchar_t Character)
{
    size_t pos=priv->TextMode*2+priv->dataChannelMode;
    if (!priv->Service[pos]->Synched)
        return;     

    priv->Service[pos]->NoChangeDuration=0;

    if (priv->Service[pos]->x==line21_Minimal_Columns)
    {
        priv->Service[pos]->x--; /* There is a problem */
    }

    if (priv->Service[pos]->InBack)
    {
        priv->Service[pos]->CC_NonDisplayed[priv->Service[pos]->y][priv->Service[pos]->x].value=Character;
        priv->Service[pos]->CC_NonDisplayed[priv->Service[pos]->y][priv->Service[pos]->x].attributes=priv->Service[pos]->Attribute_Current;
    }
    else
    {
        priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[priv->Service[pos]->y][priv->Service[pos]->x].value=Character;
        priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[priv->Service[pos]->y][priv->Service[pos]->x].attributes=priv->Service[pos]->Attribute_Current;
        ccdecoder_line21_haschanged(priv);
    }

    priv->Service[pos]->x++;
}

//---------------------------------------------------------------------------
void ccdecoder_line21_standard(ccdecoder_line21field_priv* priv, uint8_t Character)
{
    switch (Character)
    {
        //CEA-608-E, Section F.1.1.2
        case 0x20 : ccdecoder_line21_character_fill(priv, L' '     ); break;
        case 0x21 : ccdecoder_line21_character_fill(priv, L'!'     ); break;
        case 0x22 : ccdecoder_line21_character_fill(priv, L'"'     ); break;
        case 0x23 : ccdecoder_line21_character_fill(priv, L'#'     ); break;
        case 0x24 : ccdecoder_line21_character_fill(priv, L'$'     ); break;
        case 0x25 : ccdecoder_line21_character_fill(priv, L'%'     ); break;
        case 0x26 : ccdecoder_line21_character_fill(priv, L'&'     ); break;
        case 0x27 : ccdecoder_line21_character_fill(priv, L'\''    ); break;
        case 0x28 : ccdecoder_line21_character_fill(priv, L'('     ); break;
        case 0x29 : ccdecoder_line21_character_fill(priv, L')'     ); break;
        case 0x2A : ccdecoder_line21_character_fill(priv, L'\xE1'  ); break; //a acute
        case 0x2B : ccdecoder_line21_character_fill(priv, L'+'     ); break;
        case 0x2C : ccdecoder_line21_character_fill(priv, L','     ); break;
        case 0x2D : ccdecoder_line21_character_fill(priv, L'-'     ); break;
        case 0x2E : ccdecoder_line21_character_fill(priv, L'.'     ); break;
        case 0x2F : ccdecoder_line21_character_fill(priv, L'/'     ); break;
        case 0x30 : ccdecoder_line21_character_fill(priv, L'0'     ); break;
        case 0x31 : ccdecoder_line21_character_fill(priv, L'1'     ); break;
        case 0x32 : ccdecoder_line21_character_fill(priv, L'2'     ); break;
        case 0x33 : ccdecoder_line21_character_fill(priv, L'3'     ); break;
        case 0x34 : ccdecoder_line21_character_fill(priv, L'4'     ); break;
        case 0x35 : ccdecoder_line21_character_fill(priv, L'5'     ); break;
        case 0x36 : ccdecoder_line21_character_fill(priv, L'6'     ); break;
        case 0x37 : ccdecoder_line21_character_fill(priv, L'7'     ); break;
        case 0x38 : ccdecoder_line21_character_fill(priv, L'8'     ); break;
        case 0x39 : ccdecoder_line21_character_fill(priv, L'9'     ); break;
        case 0x3A : ccdecoder_line21_character_fill(priv, L':'     ); break;
        case 0x3B : ccdecoder_line21_character_fill(priv, L';'     ); break;
        case 0x3C : ccdecoder_line21_character_fill(priv, L'<'     ); break;
        case 0x3E : ccdecoder_line21_character_fill(priv, L'>'     ); break;
        case 0x3F : ccdecoder_line21_character_fill(priv, L'?'     ); break;
        case 0x40 : ccdecoder_line21_character_fill(priv, L'@'     ); break;
        case 0x41 : ccdecoder_line21_character_fill(priv, L'A'     ); break;
        case 0x42 : ccdecoder_line21_character_fill(priv, L'B'     ); break;
        case 0x43 : ccdecoder_line21_character_fill(priv, L'C'     ); break;
        case 0x44 : ccdecoder_line21_character_fill(priv, L'D'     ); break;
        case 0x45 : ccdecoder_line21_character_fill(priv, L'E'     ); break;
        case 0x46 : ccdecoder_line21_character_fill(priv, L'F'     ); break;
        case 0x47 : ccdecoder_line21_character_fill(priv, L'G'     ); break;
        case 0x48 : ccdecoder_line21_character_fill(priv, L'H'     ); break;
        case 0x49 : ccdecoder_line21_character_fill(priv, L'I'     ); break;
        case 0x4A : ccdecoder_line21_character_fill(priv, L'J'     ); break;
        case 0x4B : ccdecoder_line21_character_fill(priv, L'K'     ); break;
        case 0x4C : ccdecoder_line21_character_fill(priv, L'L'     ); break;
        case 0x4D : ccdecoder_line21_character_fill(priv, L'M'     ); break;
        case 0x4E : ccdecoder_line21_character_fill(priv, L'N'     ); break;
        case 0x4F : ccdecoder_line21_character_fill(priv, L'O'     ); break;
        case 0x50 : ccdecoder_line21_character_fill(priv, L'P'     ); break;
        case 0x51 : ccdecoder_line21_character_fill(priv, L'Q'     ); break;
        case 0x52 : ccdecoder_line21_character_fill(priv, L'R'     ); break;
        case 0x53 : ccdecoder_line21_character_fill(priv, L'S'     ); break;
        case 0x54 : ccdecoder_line21_character_fill(priv, L'T'     ); break;
        case 0x55 : ccdecoder_line21_character_fill(priv, L'U'     ); break;
        case 0x56 : ccdecoder_line21_character_fill(priv, L'V'     ); break;
        case 0x57 : ccdecoder_line21_character_fill(priv, L'W'     ); break;
        case 0x58 : ccdecoder_line21_character_fill(priv, L'X'     ); break;
        case 0x59 : ccdecoder_line21_character_fill(priv, L'Y'     ); break;
        case 0x5A : ccdecoder_line21_character_fill(priv, L'Z'     ); break;
        case 0x5B : ccdecoder_line21_character_fill(priv, L'['     ); break;
        case 0x5C : ccdecoder_line21_character_fill(priv, L'\xE9'  ); break; //e acute
        case 0x5D : ccdecoder_line21_character_fill(priv, L']'     ); break;
        case 0x5E : ccdecoder_line21_character_fill(priv, L'\xED'  ); break; //i acute
        case 0x5F : ccdecoder_line21_character_fill(priv, L'\xF3'  ); break; //o acute
        case 0x60 : ccdecoder_line21_character_fill(priv, L'\xFA'  ); break; //u acute
        case 0x61 : ccdecoder_line21_character_fill(priv, L'a'     ); break;
        case 0x62 : ccdecoder_line21_character_fill(priv, L'b'     ); break;
        case 0x63 : ccdecoder_line21_character_fill(priv, L'c'     ); break;
        case 0x64 : ccdecoder_line21_character_fill(priv, L'd'     ); break;
        case 0x65 : ccdecoder_line21_character_fill(priv, L'e'     ); break;
        case 0x66 : ccdecoder_line21_character_fill(priv, L'f'     ); break;
        case 0x67 : ccdecoder_line21_character_fill(priv, L'g'     ); break;
        case 0x68 : ccdecoder_line21_character_fill(priv, L'h'     ); break;
        case 0x69 : ccdecoder_line21_character_fill(priv, L'i'     ); break;
        case 0x6A : ccdecoder_line21_character_fill(priv, L'j'     ); break;
        case 0x6B : ccdecoder_line21_character_fill(priv, L'k'     ); break;
        case 0x6C : ccdecoder_line21_character_fill(priv, L'l'     ); break;
        case 0x6D : ccdecoder_line21_character_fill(priv, L'm'     ); break;
        case 0x6E : ccdecoder_line21_character_fill(priv, L'n'     ); break;
        case 0x6F : ccdecoder_line21_character_fill(priv, L'o'     ); break;
        case 0x70 : ccdecoder_line21_character_fill(priv, L'p'     ); break;
        case 0x71 : ccdecoder_line21_character_fill(priv, L'q'     ); break;
        case 0x72 : ccdecoder_line21_character_fill(priv, L'r'     ); break;
        case 0x73 : ccdecoder_line21_character_fill(priv, L's'     ); break;
        case 0x74 : ccdecoder_line21_character_fill(priv, L't'     ); break;
        case 0x75 : ccdecoder_line21_character_fill(priv, L'u'     ); break;
        case 0x76 : ccdecoder_line21_character_fill(priv, L'v'     ); break;
        case 0x77 : ccdecoder_line21_character_fill(priv, L'w'     ); break;
        case 0x78 : ccdecoder_line21_character_fill(priv, L'x'     ); break;
        case 0x79 : ccdecoder_line21_character_fill(priv, L'y'     ); break;
        case 0x7A : ccdecoder_line21_character_fill(priv, L'z'     ); break;
        case 0x7B : ccdecoder_line21_character_fill(priv, L'\xE7'  ); break; //c with cedilla
        case 0x7C : ccdecoder_line21_character_fill(priv, L'\xF7'  ); break; //division symbol
        case 0x7D : ccdecoder_line21_character_fill(priv, L'\xD1'  ); break; //N tilde
        case 0x7E : ccdecoder_line21_character_fill(priv, L'\xF1'  ); break; //n tilde
        case 0x7F : ccdecoder_line21_character_fill(priv, L'\x25A0'); break; //Solid block
        default   : ; //Illegal, problem
    }
}

//---------------------------------------------------------------------------
void ccdecoder_line21_preambleaddresscode(ccdecoder_line21field_priv* priv, uint8_t ccdecoder_data_1, uint8_t ccdecoder_data_2)
{
    size_t pos=priv->TextMode*2+priv->dataChannelMode;
    ccdecoder_line21_service* Service=priv->Service[pos];
    if (!Service->Synched)
        return;     

    //CEA-608-E, Section F.1.1.5

    Service->x=0; //I am not sure of this, specifications are not precise

    //Horizontal position
    if (!priv->TextMode)
    {
        Service->y=Eia608_PAC_Row[ccdecoder_data_1&0x0F]+((ccdecoder_data_2&0x20)?1:0);
        if (Service->y>=line21_Minimal_Rows)
        {
            Service->y=line21_Minimal_Rows-1;
        }

        //Clearing unused text in case of roll up mode
        if (Service->RollUpLines)
        {
            size_t pos_Y, pos_X;
            uint8_t HasChanged=0; // bool

            for (pos_Y=0; pos_Y<line21_Minimal_Rows; pos_Y++)
            {
                size_t pos=priv->TextMode*2+priv->dataChannelMode;
                if (pos_Y==((Service->y+1>Service->RollUpLines)?(Service->y+1-Service->RollUpLines):0))
                {
                    pos_Y=Service->y+1;
                    if (pos_Y>=line21_Minimal_Rows)
                        break;
                }
                for (pos_X=0; pos_X<line21_Minimal_Columns; pos_X++)
                    if (priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[pos_Y][pos_X].value!=L' ')
                    {
                        priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[pos_Y][pos_X].value=L' ';
                        priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[pos_Y][pos_X].attributes=ccdecoder_noattribute;
                        HasChanged=1;
                    }
            }
            if (HasChanged)
                ccdecoder_line21_haschanged(priv);
        }
    }

    //Attributes (except Underline)
    if (ccdecoder_data_2&0x10) //0x5x and 0x7x
    {
        Service->x=(ccdecoder_data_2&0x0E)<<1;
        Service->Attribute_Current=ccdecoder_white;
    }
    else if ((ccdecoder_data_2&0x0E)==0x0E) //0x4E, 0x4F, 0x6E, 0x6F
    {
        Service->Attribute_Current=(ccdecoder_attributes)(ccdecoder_white|ccdecoder_italic);
    }
    else //0x40-0x4D, 0x60-0x6D
        Service->Attribute_Current=(ccdecoder_attributes)((ccdecoder_data_2&0x0E)>>1);

    //Underline
    if (ccdecoder_data_2&0x01)
        Service->Attribute_Current=(ccdecoder_attributes)(Service->Attribute_Current|ccdecoder_underline);
}

//---------------------------------------------------------------------------
void ccdecoder_line21_special_10(ccdecoder_line21field_priv* priv, uint8_t cc_data_2)
{
    size_t pos=priv->TextMode*2+priv->dataChannelMode;
    ccdecoder_line21_service* Service=priv->Service[pos];
    if (!Service->Synched)
        return;     

    switch (cc_data_2)
    {
        //CEA-608-E, Section 6.2
        case 0x20 : //Background White, Opaque
        case 0x21 : //Background White, Semi-transparent
        case 0x22 : //
        case 0x23 : //
        case 0x24 : //
        case 0x25 : //
        case 0x26 : //
        case 0x27 : //
        case 0x28 : //
        case 0x29 : //
        case 0x2A : //
        case 0x2B : //
        case 0x2C : //
        case 0x2D : //
        case 0x2E : //
        case 0x2F : //
                    Service->Attribute_Current=ccdecoder_noattribute; /* Not yet supported */
        default   : ;
    }
}

//---------------------------------------------------------------------------
void ccdecoder_line21_special_11(ccdecoder_line21field_priv* priv, uint8_t cc_data_2)
{
    switch (cc_data_2)
    {
        //CEA-608-E, Section F.1.1.3
        case 0x20 : //White
        case 0x21 : //White Underline
        case 0x22 : //
        case 0x23 : //
        case 0x24 : //
        case 0x25 : //
        case 0x26 : //
        case 0x27 : //
        case 0x28 : //
        case 0x29 : //
        case 0x2A : //
        case 0x2B : //
        case 0x2C : //
        case 0x2D : //
        case 0x2E : //
        case 0x2F : //
                    {
                    size_t pos=priv->TextMode*2+priv->dataChannelMode;
                    ccdecoder_line21_service* Service=priv->Service[pos];
                    if (!Service->Synched)
                        return;     

                    //Color or Italic
                    if ((cc_data_2&0xFE)==0x2E) //Italic
                        Service->Attribute_Current=(ccdecoder_attributes)(Service->Attribute_Current|ccdecoder_italic);
                    else //Other attributes
                        Service->Attribute_Current=(ccdecoder_attributes)((cc_data_2&0x0F)>>1);

                    //Underline
                    if (cc_data_2&0x01)
                        Service->Attribute_Current=(ccdecoder_attributes)(Service->Attribute_Current|ccdecoder_underline);
                    }

                    break;
        //CEA-608-E, Section F.1.1.1
        case 0x30 : ccdecoder_line21_character_fill(priv, L'\x2122'); break;  //Registered mark symbol
        case 0x31 : ccdecoder_line21_character_fill(priv, L'\xB0'  ); break;  //Degree sign
        case 0x32 : ccdecoder_line21_character_fill(priv, L'\xBD'  ); break;  //1/2
        case 0x33 : ccdecoder_line21_character_fill(priv, L'\xBF'  ); break;  //interogation mark inverted
        case 0x34 : ccdecoder_line21_character_fill(priv, L'\xA9'  ); break;  //Trademark symbol
        case 0x35 : ccdecoder_line21_character_fill(priv, L'\xA2'  ); break;  //Cents sign
        case 0x36 : ccdecoder_line21_character_fill(priv, L'\xA3'  ); break;  //Pounds Sterling sign
        case 0x37 : ccdecoder_line21_character_fill(priv, L'\x266A'); break;  //Music note
        case 0x38 : ccdecoder_line21_character_fill(priv, L'\xE0'  ); break;  //a grave
        case 0x39 : ccdecoder_line21_character_fill(priv, L' '     ); break;  //Transparent space
        case 0x3A : ccdecoder_line21_character_fill(priv, L'\xE8'  ); break;  //e grave
        case 0x3B : ccdecoder_line21_character_fill(priv, L'\xE2'  ); break;  //a circumflex
        case 0x3C : ccdecoder_line21_character_fill(priv, L'\xEA'  ); break;  //e circumflex
        case 0x3D : ccdecoder_line21_character_fill(priv, L'\xEE'  ); break;  //i circumflex
        case 0x3E : ccdecoder_line21_character_fill(priv, L'\xF4'  ); break;  //o circumflex
        case 0x3F : ccdecoder_line21_character_fill(priv, L'\xFB'  ); break;  //u circumflex
        default   : ;
    }
}

//---------------------------------------------------------------------------
void ccdecoder_line21_special_12(ccdecoder_line21field_priv* priv, uint8_t cc_data_2)
{
    size_t pos=priv->TextMode*2+priv->dataChannelMode;
    ccdecoder_line21_service* Service=priv->Service[pos];
    if (!Service->Synched)
        return;     
    
   if (priv->Service[pos]->x && cc_data_2>=0x20 && cc_data_2<0x40)
        priv->Service[pos]->x--; //Erasing previous character
        
   switch (cc_data_2)
    {
        //CEA-608-E, Section 6.4.2
        case 0x20 : ccdecoder_line21_character_fill(priv, L'A'     ); break;  //A with acute
        case 0x21 : ccdecoder_line21_character_fill(priv, L'E'     ); break;  //E with acute
        case 0x22 : ccdecoder_line21_character_fill(priv, L'O'     ); break;  //O with acute
        case 0x23 : ccdecoder_line21_character_fill(priv, L'U'     ); break;  //U with acute
        case 0x24 : ccdecoder_line21_character_fill(priv, L'U'     ); break;  //U withdiaeresis or umlaut
        case 0x25 : ccdecoder_line21_character_fill(priv, L'u'     ); break;  //u with diaeresis or umlaut
        case 0x26 : ccdecoder_line21_character_fill(priv, L'\''    ); break;  //opening single quote
        case 0x27 : ccdecoder_line21_character_fill(priv, L'!'     ); break;  //inverted exclamation mark
        case 0x28 : ccdecoder_line21_character_fill(priv, L'*'     ); break;  //Asterisk
        case 0x29 : ccdecoder_line21_character_fill(priv, L'\''    ); break;  //plain single quote
        case 0x2A : ccdecoder_line21_character_fill(priv, L'_'     ); break;  //em dash
        case 0x2B : ccdecoder_line21_character_fill(priv, L'C'     ); break;  //Copyright
        case 0x2C : ccdecoder_line21_character_fill(priv, L'S'     ); break;  //Servicemark
        case 0x2D : ccdecoder_line21_character_fill(priv, L'x'     ); break;  //round bullet
        case 0x2E : ccdecoder_line21_character_fill(priv, L'\"'    ); break;  //opening double quotes
        case 0x2F : ccdecoder_line21_character_fill(priv, L'\"'    ); break;  //closing double quotes
        case 0x30 : ccdecoder_line21_character_fill(priv, L'A'     ); break;  //A with grave accent
        case 0x31 : ccdecoder_line21_character_fill(priv, L'A'     ); break;  //A with circumflex accent
        case 0x32 : ccdecoder_line21_character_fill(priv, L'C'     ); break;  //C with cedilla
        case 0x33 : ccdecoder_line21_character_fill(priv, L'E'     ); break;  //E with grave accent
        case 0x34 : ccdecoder_line21_character_fill(priv, L'E'     ); break;  //E with circumflex accent
        case 0x35 : ccdecoder_line21_character_fill(priv, L'E'     ); break;  //E with diaeresis or umlaut mark
        case 0x36 : ccdecoder_line21_character_fill(priv, L'e'     ); break;  //e with diaeresis or umlaut mark
        case 0x37 : ccdecoder_line21_character_fill(priv, L'I'     ); break;  //I with circumflex accent
        case 0x38 : ccdecoder_line21_character_fill(priv, L'I'     ); break;  //I with diaeresis or umlaut mark
        case 0x39 : ccdecoder_line21_character_fill(priv, L'i'     ); break;  //i with diaeresis or umlaut mark
        case 0x3A : ccdecoder_line21_character_fill(priv, L'O'     ); break;  //O with circumflex
        case 0x3B : ccdecoder_line21_character_fill(priv, L'U'     ); break;  //U with grave accent
        case 0x3C : ccdecoder_line21_character_fill(priv, L'u'     ); break;  //u with grave accent
        case 0x3D : ccdecoder_line21_character_fill(priv, L'U'     ); break;  //U with circumflex accent
        case 0x3E : ccdecoder_line21_character_fill(priv, L'\"'    ); break;  //opening guillemets
        case 0x3F : ccdecoder_line21_character_fill(priv, L'\"'    ); break;  //closing guillemets
        default   : ;
    }
}

//---------------------------------------------------------------------------
void ccdecoder_line21_special_13(ccdecoder_line21field_priv* priv, uint8_t cc_data_2)
{
    size_t pos=priv->TextMode*2+priv->dataChannelMode;
    ccdecoder_line21_service* Service=priv->Service[pos];
    if (!Service->Synched)
        return;     
    
   if (priv->Service[pos]->x && cc_data_2>=0x20 && cc_data_2<0x40)
        priv->Service[pos]->x--; //Erasing previous character
        
    switch (cc_data_2)
    {
        //CEA-608-E, Section 6.4.2
        case 0x20 : ccdecoder_line21_character_fill(priv, L'A'     ); break;  //A with tilde
        case 0x21 : ccdecoder_line21_character_fill(priv, L'a'     ); break;  //a with tilde
        case 0x22 : ccdecoder_line21_character_fill(priv, L'I'     ); break;  //I with acute accent
        case 0x23 : ccdecoder_line21_character_fill(priv, L'I'     ); break;  //I with grave accent
        case 0x24 : ccdecoder_line21_character_fill(priv, L'i'     ); break;  //i with grave accent
        case 0x25 : ccdecoder_line21_character_fill(priv, L'O'     ); break;  //O with grave accent
        case 0x26 : ccdecoder_line21_character_fill(priv, L'o'     ); break;  //o with grave accent
        case 0x27 : ccdecoder_line21_character_fill(priv, L'O'     ); break;  //O with tilde
        case 0x28 : ccdecoder_line21_character_fill(priv, L'o'     ); break;  //o with tilde
        case 0x29 : ccdecoder_line21_character_fill(priv, L'{'     ); break;  //opening brace
        case 0x2A : ccdecoder_line21_character_fill(priv, L'}'     ); break;  //closing brace
        case 0x2B : ccdecoder_line21_character_fill(priv, L'\\'    ); break;  //backslash
        case 0x2C : ccdecoder_line21_character_fill(priv, L'^'     ); break;  //caret
        case 0x2D : ccdecoder_line21_character_fill(priv, L'_'     ); break;  //Underbar
        case 0x2E : ccdecoder_line21_character_fill(priv, L'|'     ); break;  //pipe
        case 0x2F : ccdecoder_line21_character_fill(priv, L'~'     ); break;  //tilde
        case 0x30 : ccdecoder_line21_character_fill(priv, L'A'     ); break;  //A with diaeresis or umlaut mark
        case 0x31 : ccdecoder_line21_character_fill(priv, L'a'     ); break;  //a with diaeresis or umlaut mark
        case 0x32 : ccdecoder_line21_character_fill(priv, L'O'     ); break;  //o with diaeresis or umlaut mark
        case 0x33 : ccdecoder_line21_character_fill(priv, L'o'     ); break;  //o with diaeresis or umlaut mark
        case 0x34 : ccdecoder_line21_character_fill(priv, L's'     ); break;  //eszett (mall sharp s)
        case 0x35 : ccdecoder_line21_character_fill(priv, L'Y'     ); break;  //yen
        case 0x36 : ccdecoder_line21_character_fill(priv, L' '     ); break;  //non-specific currency sign
        case 0x37 : ccdecoder_line21_character_fill(priv, L'|'     ); break;  //Vertical bar
        case 0x38 : ccdecoder_line21_character_fill(priv, L'A'     ); break;  //I with diaeresis or umlaut mark
        case 0x39 : ccdecoder_line21_character_fill(priv, L'a'     ); break;  //i with diaeresis or umlaut mark
        case 0x3A : ccdecoder_line21_character_fill(priv, L'O'     ); break;  //O with ring
        case 0x3B : ccdecoder_line21_character_fill(priv, L'o'     ); break;  //a with ring
        case 0x3C : ccdecoder_line21_character_fill(priv, L' '     ); break;  //upper left corner
        case 0x3D : ccdecoder_line21_character_fill(priv, L' '     ); break;  //upper right corner
        case 0x3E : ccdecoder_line21_character_fill(priv, L' '     ); break;  //lower left corner
        case 0x3F : ccdecoder_line21_character_fill(priv, L' '     ); break;  //lower right corner
        default   : ;
    }
}

//---------------------------------------------------------------------------
void ccdecoder_line21_special_14(ccdecoder_line21field_priv* priv, uint8_t ccdecoder_data_2)
{
    size_t pos, pos_X, pos_Y;
    ccdecoder_line21_service* Service;
    int HasChanged;
    
    switch (ccdecoder_data_2)
    {
        case 0x20 : //RCL - Resume Caption Loading
        case 0x25 : //RU2 - Roll-Up Captions–2 Rows
        case 0x26 : //RU3 - Roll-Up Captions–3 Rows
        case 0x27 : //RU4 - Roll-Up Captions–4 Rows
        case 0x29 : //RDC - Resume Direct Captioning
        case 0x2A : //TR  - Text Restart
        case 0x2B : //RTD - Resume Text Display
                    priv->TextMode=ccdecoder_data_2>=0x2A;
                    priv->XDS_Level=(size_t)-1; // No more XDS
                    pos=priv->TextMode*2+priv->dataChannelMode;
                    priv->Service[pos]->Synched=1; // With have a point of synchro

                    // Alloc
                    if (priv->CC->transports[0]->captions==NULL)
                    {
                        priv->CC->transports[0]->captions=ccdecoder_captions_init();
                        if (priv->CC->transports[0]->captions==NULL)
                            return; // Problem
                    }
                    if (priv->CC->transports[0]->captions[priv->CC_Offset+pos]==NULL)
                    {
                        priv->CC->transports[0]->captions[priv->CC_Offset+pos]=ccdecoder_caption_init();
                        if (priv->CC->transports[0]->captions[priv->CC_Offset+pos]==NULL)
                            return; // Problem
                    }
                    if (priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal==NULL)
                    {
                        priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal=ccdecoder_line21_alloc_display_alloc();
                        if (priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal==NULL)
                            return; // Problem
                    }
                    if (priv->CC_Offset+pos>=priv->CC->transports[0]->count)
                        priv->CC->transports[0]->count=priv->CC_Offset+pos+1;
                    break; //This is Caption or Text
        case 0x2F : //EOC - end of Caption
                    {
                    priv->TextMode=0;
                    pos=priv->TextMode*2+priv->dataChannelMode;
                    }
                    break; //This is end of Caption or Text
        default: ;
    }

    pos=priv->TextMode*2+priv->dataChannelMode;

    if (pos>=4 || priv->Service[pos]==NULL || (!priv->Service[pos]->Synched && ccdecoder_data_2!=0x2C && ccdecoder_data_2!=0x2E))
        return; //Not synched

    Service=priv->Service[pos];
    switch (ccdecoder_data_2)
    {
        case 0x20 : Service->InBack=1;
                    break; //RCL - Resume Caption Loading (Select pop-on style)
        case 0x21 : if (Service->x)
                        Service->x--;
                    if (priv->CC->transports[0] && priv->CC->transports[0]->captions && priv->CC->transports[0]->captions[priv->CC_Offset+pos] && priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal)
                    {
                        (Service->InBack?Service->CC_NonDisplayed:priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal)[Service->y][Service->x].value=L' '; //Clear the character
                        (Service->InBack?Service->CC_NonDisplayed:priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal)[Service->y][Service->x].attributes=ccdecoder_noattribute; //Clear the character
                        if (!Service->InBack)
                            ccdecoder_line21_haschanged(priv);
                    }
                    break; //BS  - Backspace
        case 0x22 : ccdecoder_line21_special_14(priv, 0x2D); //Found 1 file with AOF and non CR
                    break; //AOF - Alarm Off
        case 0x23 : break; //AON - Alarm On
        case 0x24 : 
                    if (priv->CC->transports[0] && priv->CC->transports[0]->captions && priv->CC->transports[0]->captions[priv->CC_Offset+pos] && priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal)
                    {
                        for (pos_X=Service->x; pos_X<line21_Minimal_Columns; pos_X++)
                        {
                            (Service->InBack?Service->CC_NonDisplayed:priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal)[Service->y][pos_X].value=L' '; //Clear up to the end of line
                            (Service->InBack?Service->CC_NonDisplayed:priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal)[Service->y][pos_X].attributes=ccdecoder_noattribute; //Clear up to the end of line
                        }
                        if (!Service->InBack)
                            ccdecoder_line21_haschanged(priv);
                    }
                    break; //DER - Delete to end of Row
        case 0x25 : //RU2 - Roll-Up Captions-2 Rows
        case 0x26 : //RU3 - Roll-Up Captions-3 Rows
        case 0x27 : //RU4 - Roll-Up Captions-4 Rows
                    Service->RollUpLines=ccdecoder_data_2-0x25+2;
                    Service->InBack=0;
                    break; //RUx - Roll-Up Captions–x Rows
        case 0x28 : break; //FON - Flash On
        case 0x29 : Service->InBack=0;
                    break; //RDC - Resume Direct Captioning (paint-on style)
        case 0x2A : Service->RollUpLines=line21_Minimal_Rows; //Roll up all the lines
                    Service->y=line21_Minimal_Rows-1; //Base is the bottom line
                    Service->Attribute_Current=ccdecoder_noattribute; //Reset all attributes
                    ccdecoder_line21_special_14(priv, 0x2D); //Next line
                    break; //TR  - Text Restart (clear Text, but not boxes)
        case 0x2B : break; //RTD - Resume Text Display
        case 0x2C :
                    if (priv->CC->transports[0] && priv->CC->transports[0]->captions && priv->CC->transports[0]->captions[priv->CC_Offset+pos] && priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal)
                    {
                        HasChanged=0;
                        for (pos_Y=0; pos_Y<line21_Minimal_Rows; pos_Y++)
                            for (pos_X=0; pos_X<line21_Minimal_Columns; pos_X++)
                                if (priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[pos_Y][pos_X].value!=L' ')
                                {
                                    priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[pos_Y][pos_X].value=L' ';
                                    priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[pos_Y][pos_X].attributes=ccdecoder_noattribute;
                                    HasChanged=1;
                                }
                        if (HasChanged)
                            ccdecoder_line21_haschanged(priv);
                    }
                    priv->Service[pos]->NoChangeDuration=(size_t)-1;
                    break; //EDM - Erase Displayed Memory
        case 0x2D : 
                    if (priv->CC->transports[0] && priv->CC->transports[0]->captions && priv->CC->transports[0]->captions[priv->CC_Offset+pos] && priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal)
                    {
                        for (pos_X=1; pos_X<Service->RollUpLines; pos_X++)
                        {
                            if (Service->y>=Service->RollUpLines-pos && Service->y-Service->RollUpLines+pos+1<line21_Minimal_Rows)
                            {
                                ccdecoder_line21_alloc_display_line_free(priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[Service->y-Service->RollUpLines+pos_X]);
                                priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[Service->y-Service->RollUpLines+pos_X]=priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[Service->y-Service->RollUpLines+pos_X+1];
                                priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[Service->y-Service->RollUpLines+pos_X+1]=ccdecoder_line21_alloc_display_line_alloc();
                            }
                        }
                        if (!Service->InBack)
                            ccdecoder_line21_haschanged(priv);
                    }
                    Service->x=0;
                    break; //CR  - Carriage Return
        case 0x2E : if (Service->CC_NonDisplayed)
                        for (pos_Y=0; pos_Y<line21_Minimal_Rows; pos_Y++)
                            for (pos_X=0; pos_X<line21_Minimal_Columns; pos_X++)
                            {
                                Service->CC_NonDisplayed[pos_Y][pos_X].value=L' ';
                                Service->CC_NonDisplayed[pos_Y][pos_X].attributes=ccdecoder_noattribute;
                            }
                    break; //ENM - Erase Non-Displayed Memory
        case 0x2F : 
                    if (priv->CC->transports[0] && priv->CC->transports[0]->captions && priv->CC->transports[0]->captions[priv->CC_Offset+pos] && priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal)
                    {
                        ccdecoder_line21_alloc_display_free(priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal);
                        priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal=Service->CC_NonDisplayed;
                        Service->CC_NonDisplayed=ccdecoder_line21_alloc_display_alloc();
                        ccdecoder_line21_haschanged(priv);
                    }
                    priv->Service[pos]->Synched=0;
                    break; //EOC - end of Caption
        default   : ; //Illegal, problem
    }
}

void ccdecoder_line21_special_17(ccdecoder_line21field_priv* priv, uint8_t cc_data_2)
{
    size_t pos=priv->TextMode*2+priv->dataChannelMode;
    ccdecoder_line21_service* Service=priv->Service[pos];

    switch (cc_data_2)
    {
        //CEA-608-E, section B.4
        case 0x21 : //TO1 - Tab offset 1 Column
        case 0x22 : //TO2 - Tab offset 2 Columns
        case 0x23 : //TO3 - Tab offset 3 Columns
                    Service->x+=cc_data_2&0x03;
                    if (Service->x>=line21_Minimal_Columns)
                        Service->x=line21_Minimal_Columns-1;
                    break;
        //CEA-608-E, section 6.3
        case 0x24 : break;  //Select the standard line 21 character set in normal size
        case 0x25 : break;  //Select the standard line 21 character set in double size
        case 0x26 : break;  //Select the first priv character set
        case 0x27 : break;  //Select the second priv character set
        case 0x28 : break;  //Select the People's Republic of China character set: GB 2312-80
        case 0x29 : break;  //Select the Korean Standard character set: KSC 5601-1987
        case 0x2A : break;  //Select the first registered character set
        //CEA-608-E, section 6.2
        case 0x2D : break;  //Background Transparent
        case 0x2E : break;  //Foreground Black
        case 0x2F : break;  //Foreground Black Underline
        default   : ;
    }
}


//---------------------------------------------------------------------------
void ccdecoder_line21_special(ccdecoder_line21field_priv* priv, uint8_t ccdecoder_data_1, uint8_t ccdecoder_data_2)
{
    //Saving data, for repetition of the code
    priv->ccdecoder_data_1_Old=ccdecoder_data_1;
    priv->ccdecoder_data_2_Old=ccdecoder_data_2;

    //data channel check
    priv->dataChannelMode=(ccdecoder_data_1&0x08)!=0; //bit3 is the data Channel number

    //Field check
    switch (priv->FieldNumber)
    {
        case 0 :
                if ((ccdecoder_data_1==0x15 || ccdecoder_data_1==0x1D) && (ccdecoder_data_2&0xF0)==0x20)
                    return; /* Problem */
                break;
        case 1 :
                if ((ccdecoder_data_1==0x14 || ccdecoder_data_1==0x1C) && (ccdecoder_data_2&0xF0)==0x20)
                    return; /* Problem */
                break;
        default:
                if ((ccdecoder_data_1==0x14 || ccdecoder_data_1==0x1C) && (ccdecoder_data_2&0xF0)==0x20)
                    priv->FieldNumber=0; /* Synching on 1st field */
                if ((ccdecoder_data_1==0x15 || ccdecoder_data_1==0x1D) && (ccdecoder_data_2&0xF0)==0x20)
                    priv->FieldNumber=1; /* Synching on 2nd field */
    }

    ccdecoder_data_1&=0xF7;
    if (ccdecoder_data_1==0x15 && (ccdecoder_data_2&0xF0)==0x20)
        ccdecoder_data_1=0x14;

    if (ccdecoder_data_1>=0x10 && ccdecoder_data_1<=0x17 && ccdecoder_data_2>=0x40)
    {
        ccdecoder_line21_preambleaddresscode(priv, ccdecoder_data_1, ccdecoder_data_2);
    }
    else
    {
        switch (ccdecoder_data_1)
        {
            case 0x10 : ccdecoder_line21_special_10(priv, ccdecoder_data_2); break;
            case 0x11 : ccdecoder_line21_special_11(priv, ccdecoder_data_2); break;
            case 0x12 : ccdecoder_line21_special_12(priv, ccdecoder_data_2); break;
            case 0x13 : ccdecoder_line21_special_13(priv, ccdecoder_data_2); break;
            case 0x14 : ccdecoder_line21_special_14(priv, ccdecoder_data_2); break;
            case 0x17 : ccdecoder_line21_special_17(priv, ccdecoder_data_2); break;
            default   : ; // Illegal, problem
        }
    }
}

//---------------------------------------------------------------------------
void ccdecoder_line21_xds_item_parse(ccdecoder_line21field_priv* priv)
{
    if (priv->XDS[priv->XDS_Level]->size<4)
    {
        priv->XDS[priv->XDS_Level]->size=0;
        return; //There is a problem
    }

    //switch (priv->XDS[priv->XDS_Level]->data[0])
    //{
    //    default   : ; // XDS is currently not decoded
    //}

    priv->XDS[priv->XDS_Level]->size=0;
}

//---------------------------------------------------------------------------
void ccdecoder_line21_xds_parse(ccdecoder_line21field_priv* priv, uint8_t ccdecoder_data_1, uint8_t ccdecoder_data_2)
{
    if (ccdecoder_data_1 && ccdecoder_data_1<0x10 && ccdecoder_data_1%2==0)
    {
        // Continue
        ccdecoder_data_1--;
        for (priv->XDS_Level=0; priv->XDS_Level<8; priv->XDS_Level++)
            if (priv->XDS[priv->XDS_Level]->size>=2 && priv->XDS[priv->XDS_Level]->data[0]==ccdecoder_data_1 && priv->XDS[priv->XDS_Level]->data[1]==ccdecoder_data_2)
                break;
        if (priv->XDS_Level>=8)
            priv->XDS_Level=(size_t)-1; // There is a problem

        return;
    }
    else if (ccdecoder_data_1 && ccdecoder_data_1<0x0F)
    {
        // Start
        for (priv->XDS_Level=0; priv->XDS_Level<8; priv->XDS_Level++)
            if (priv->XDS[priv->XDS_Level]->size>=2 && priv->XDS[priv->XDS_Level]->data[0]==ccdecoder_data_1 && priv->XDS[priv->XDS_Level]->data[1]==ccdecoder_data_2)
                break;
        if (priv->XDS_Level>=8)
        {
            for (priv->XDS_Level=0; priv->XDS_Level<8; priv->XDS_Level++)
                if (priv->XDS[priv->XDS_Level]->size==0)
                    break;
            if (priv->XDS_Level>=8)
            {
                // There is a problem, not enough buffers, erasing all
                for (priv->XDS_Level=0; priv->XDS_Level<8; priv->XDS_Level++)
                    priv->XDS[priv->XDS_Level]->size=0;
                priv->XDS_Level=0; 
            }
        }
        else
            priv->XDS[priv->XDS_Level]->size=0; // There is a problem, erasing the previous item
    }
    
    if (priv->XDS_Level==(size_t)-1)
        return; //There is a problem

    priv->XDS[priv->XDS_Level]->data[priv->XDS[priv->XDS_Level]->size]=ccdecoder_data_1;
    priv->XDS[priv->XDS_Level]->data[priv->XDS[priv->XDS_Level]->size+1]=ccdecoder_data_2;
    priv->XDS[priv->XDS_Level]->size+=2;
    if (ccdecoder_data_1==0x0F)
        ccdecoder_line21_xds_item_parse(priv);
    if (priv->XDS[priv->XDS_Level]->size>=36)
        priv->XDS[priv->XDS_Level]->size=0; // Clear, this is a security
    priv->TextMode=0; // This is CC
}

size_t ccdecoder_line21field_parse (ccdecoder_handle* handle, uint8_t* data, size_t size, size_t options, double pts, double dts, double dur)
{
    size_t pos;
    uint8_t ccdecoder_data_1;
    uint8_t ccdecoder_data_2;
    ccdecoder_line21field_priv* priv;

    if (handle==NULL)
        return ccdecoder_nochange;
    if (ccdecoder_internal_parse_begin(handle, data, size, options, 0x30))
        return ccdecoder_memoryerror;

    priv=(ccdecoder_line21field_priv*)handle->priv;

    if (options&ccdecoder_unsynched)
    {
        for (pos=0; pos<4; pos++)
            priv->Service[pos]->Synched=0;
        for (pos=0; pos<8; pos++)
            priv->XDS[pos]->size=0;
        priv->XDS_Level=(size_t)-1;
    }

    if (size!=2)
        return 0; // Stream method not yet supported
    
    // Removing checksum
    ccdecoder_data_1=data[0]&0x7F;
    ccdecoder_data_2=data[1]&0x7F;

    // Test if non-printing chars (0x10-0x1F) are repeated (CEA-608-E section D.2)
    if (priv->ccdecoder_data_1_Old)
    {
        if (priv->ccdecoder_data_1_Old==ccdecoder_data_1 && priv->ccdecoder_data_2_Old==ccdecoder_data_2)
        {
            // This is duplicate
            priv->ccdecoder_data_1_Old=0x00;
            priv->ccdecoder_data_2_Old=0x00;
            return 0; // Nothing to do
        }
        else
        {
            // They should be duplicated, there is a problem
        }
        priv->ccdecoder_data_1_Old=0x00;
        priv->ccdecoder_data_2_Old=0x00;
    }
    
    if ((ccdecoder_data_1 && ccdecoder_data_1<0x10) || (priv->XDS_Level!=(size_t)-1 && ccdecoder_data_1>=0x20)) //XDS
        ccdecoder_line21_xds_parse(priv, ccdecoder_data_1, ccdecoder_data_2);
    else if (ccdecoder_data_1>=0x20) // Basic characters
    {
        size_t pos=priv->TextMode*2+priv->dataChannelMode;
        ccdecoder_line21_service* Service=priv->Service[pos];
        if (Service->Synched)
        {
            ccdecoder_line21_standard(priv, ccdecoder_data_1);
            if ((ccdecoder_data_2&0x7F)>=0x20)
                ccdecoder_line21_standard(priv, ccdecoder_data_2);
        }
    }
    else if (ccdecoder_data_1) // Special
        ccdecoder_line21_special(priv, ccdecoder_data_1, ccdecoder_data_2);
    // Detection of too long caption duration
    for (pos=0; pos<4; pos++)
        if (priv->CC->transports[0] && priv->CC->transports[0]->captions && priv->CC->transports[0]->captions[priv->CC_Offset+pos] && priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal && priv->Service[pos]->Synched && priv->Service[pos]->NoChangeDuration!=(size_t)-1)
        {
            priv->Service[pos]->NoChangeDuration++;
            if (priv->Service[pos]->NoChangeDuration>=500) // ~16 seconds at 30 fps
            {
                //EDM - Erase Displayed Memory
                size_t HasChanged=0, pos_Y, pos_X;
                for (pos_Y=0; pos_Y<line21_Minimal_Rows; pos_Y++)
                    for (pos_X=0; pos_X<line21_Minimal_Columns; pos_X++)
                        if (priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[pos_Y][pos_X].value!=L' ')
                        {
                            priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[pos_Y][pos_X].value=L' ';
                            priv->CC->transports[0]->captions[priv->CC_Offset+pos]->minimal[pos_Y][pos_X].attributes=ccdecoder_noattribute;
                            HasChanged=1;
                        }
                if (HasChanged)
                    ccdecoder_line21_haschanged(priv);
                priv->Service[pos]->NoChangeDuration=(size_t)-1;
            }
        }

    return ccdecoder_line21_returnvalue(priv);
}

void  ccdecoder_line21field_free  (ccdecoder_handle* handle)
{
    size_t pos;
    ccdecoder_line21field_priv* priv;

    if (handle==NULL)
        return;

    priv=(ccdecoder_line21field_priv*)handle->priv;
    if (priv==NULL)
        return;

    if (priv->CC->transports[0]->captions)
        for (pos=0; pos<4; pos++)
            if (priv->CC->transports[0]->captions[pos])
            {
                ccdecoder_line21_alloc_display_free(priv->CC->transports[0]->captions[pos]->minimal); priv->CC->transports[0]->captions[pos]->minimal=NULL;
            }
    ccdecoder_line21_service_free(priv->Service[0]);
    ccdecoder_line21_service_free(priv->Service[1]);
    ccdecoder_line21_service_free(priv->Service[2]);
    ccdecoder_line21_service_free(priv->Service[3]);
    for (pos=0; pos<8; pos++)
        free(priv->XDS[pos]);

    ccdecoder_internal_free(handle, 0x30);

    free(priv);
}
