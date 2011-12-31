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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <wchar.h>
#include "ccdecoder_subrip.h"
#include "ccdecoder.h"
#ifdef CCDECODER_DUMP
    #include <stdio.h>
#endif //CCDECODER_DUMP

typedef struct ccdecoder_subrip_handle_priv
{
    size_t                          Number;
    size_t                          MaxLine;
    size_t                          MaxColumn;
    ccdecoder_minimal_character**   Characters;
    double                          pts;
    double                          pts_begin;
    int                             Hascontent; // bool
} ccdecoder_subrip_handle_priv;

void to_timestamp(double Value_Double, wchar_t Result[20])
{
    size_t pos=0;
    int64_t value;
    int64_t Hours, Minutes, Seconds;

    if (Value_Double==DBL_MAX)
    {
        memcpy(Result, L"??:??:??.???", 13*sizeof(wchar_t));
        return;
    }
    value=(int64_t)(Value_Double*1000);

    /* Hours */
    Hours=value/(60*60*1000);
    if (Hours>=1000)
        return; /* Not supported */
    if (Hours>=100)
    {
        Result[pos]=L'0'+(wchar_t)Hours/100;
        pos++;
        value-=Hours*(100*60*60*1000);
        Hours%=100;
    }
    Result[pos]=L'0'+(wchar_t)Hours/10;
    pos++;
    Result[pos]=L'0'+(wchar_t)Hours%10;
    pos++;
    value-=Hours*(60*60*1000);
    Result[pos]=L':';
    pos++;

    /* Minutes */
    Minutes=value/(60*1000);
    Result[pos]=L'0'+(wchar_t)Minutes/10;
    pos++;
    Result[pos]=L'0'+(wchar_t)Minutes%10;
    pos++;
    value-=Minutes*(60*1000);
    Result[pos]=L':';
    pos++;

    /* Seconds */
    Seconds=value/1000;
    Result[pos]=L'0'+(wchar_t)Seconds/10;
    pos++;
    Result[pos]=L'0'+(wchar_t)Seconds%10;
    pos++;
    value-=Seconds*1000;
    Result[pos]=L',';
    pos++;

    /* Milliseconds */
    Result[pos]=L'0'+(wchar_t)value/100;
    pos++;
    Result[pos]=L'0'+(wchar_t)(value%100)/10;
    pos++;
    Result[pos]=L'0'+(wchar_t)value%10;
    pos++;
    Result[pos]=L'\0';
}

ccdecoder_subrip_handle* ccdecoder_subrip_alloc  ()
{
    ccdecoder_subrip_handle*            handle;
    ccdecoder_subrip_handle_priv*       priv;

    // Common part
    handle=(ccdecoder_subrip_handle*)malloc(sizeof(ccdecoder_subrip_handle));
    if (handle==NULL)
        return NULL;

    // Private part
    handle->priv=priv=(ccdecoder_subrip_handle_priv*)malloc(sizeof(ccdecoder_subrip_handle_priv));
    handle->pts_in=DBL_MAX;
    handle->pts_out=DBL_MAX;

    priv->Number=0;
    priv->Characters=NULL;
    priv->pts=DBL_MAX;
    priv->pts_begin=DBL_MAX;
    priv->Hascontent=0;

    return handle;
}

wchar_t* ccdecoder_subrip_parse(ccdecoder_subrip_handle* handle, ccdecoder_caption* caption, double pts)
{
    ccdecoder_subrip_handle_priv*       priv;
    wchar_t*                            ToReturn=NULL;
    wchar_t*                            ToReturn_Current;
    size_t                              ToReturn_size=0, MaxLine=0, MaxColumn=0, Line, Column, Y_Min=(size_t)-1, Y_Max=(size_t)-1;

    #ifdef CCDECODER_DUMP
        fprintf(stderr, "ccdecoder subrip: ");
    #endif //CCDECODER_DUMP

    if (caption==NULL || !(caption->status&ccdecoder_haschanged) || !caption->minimal)
        return NULL; /* Problem */
    
    // Common part
    if (handle==NULL)
        return NULL;

    // Private part
    priv=(ccdecoder_subrip_handle_priv*)handle->priv;
           
    /* Releasing previous screen (if we have start and end timestamp = 2nd call) */
    if (priv->Hascontent) /* If some content before */
    {
        wchar_t pts_In[20], pts_Out[20];
        size_t Output_Y, Output_X;

        /* Init of returned string */
        ToReturn_size=100+priv->MaxLine*(priv->MaxColumn+1); /* 100 for extra data, +1 for \n */
        ToReturn=ToReturn_Current=(wchar_t*)malloc(ToReturn_size*sizeof(wchar_t));
        ToReturn_Current[0]=L'\0';

        to_timestamp(priv->pts, pts_In);
        to_timestamp(pts, pts_Out);
        swprintf(ToReturn_Current, ToReturn_size, L"%i\n", (int)priv->Number); ToReturn_Current+=wcslen(ToReturn_Current);
        swprintf(ToReturn_Current, ToReturn_size, L"%ls --> %ls\n", pts_In, pts_Out); ToReturn_Current+=wcslen(ToReturn_Current);
        for (Output_Y=0; priv->Characters[Output_Y][0].value; Output_Y++)
        {
            uint8_t attributes=ccdecoder_noattribute;
            for (Output_X=0; priv->Characters[Output_Y][Output_X].value; Output_X++)
            {
                /* Attributes */
                if (attributes!=priv->Characters[Output_Y][Output_X].attributes)
                {
                    if (attributes&ccdecoder_underline)
                    {
                        *ToReturn_Current=L'<'; ToReturn_Current++;
                        *ToReturn_Current=L'/'; ToReturn_Current++;
                        *ToReturn_Current=L'u'; ToReturn_Current++;
                        *ToReturn_Current=L'>'; ToReturn_Current++;
                    }
                    if (attributes&ccdecoder_italic)
                    {
                        *ToReturn_Current=L'<'; ToReturn_Current++;
                        *ToReturn_Current=L'/'; ToReturn_Current++;
                        *ToReturn_Current=L'i'; ToReturn_Current++;
                        *ToReturn_Current=L'>'; ToReturn_Current++;
                    }

                    attributes=priv->Characters[Output_Y][Output_X].attributes;

                    if (attributes&ccdecoder_italic)
                    {
                        *ToReturn_Current=L'<'; ToReturn_Current++;
                        *ToReturn_Current=L'i'; ToReturn_Current++;
                        *ToReturn_Current=L'>'; ToReturn_Current++;
                    }
                    if (attributes&ccdecoder_underline)
                    {
                        *ToReturn_Current=L'<'; ToReturn_Current++;
                        *ToReturn_Current=L'u'; ToReturn_Current++;
                        *ToReturn_Current=L'>'; ToReturn_Current++;
                    }
                }

                /* Value */
                *ToReturn_Current=priv->Characters[Output_Y][Output_X].value; ToReturn_Current++;
            }

            /* Attributes (at end of the line) */
            if (attributes)
            {
                if (attributes&ccdecoder_underline)
                {
                    *ToReturn_Current=L'<'; ToReturn_Current++;
                    *ToReturn_Current=L'/'; ToReturn_Current++;
                    *ToReturn_Current=L'u'; ToReturn_Current++;
                    *ToReturn_Current=L'>'; ToReturn_Current++;
                }
                if (attributes&ccdecoder_italic)
                {
                    *ToReturn_Current=L'<'; ToReturn_Current++;
                    *ToReturn_Current=L'/'; ToReturn_Current++;
                    *ToReturn_Current=L'i'; ToReturn_Current++;
                    *ToReturn_Current=L'>'; ToReturn_Current++;
                }
            }

            /* Carriage return */
            *ToReturn_Current=L'\n'; ToReturn_Current++;
        }

        /* Additional carriage return and \0 at the end */
        *ToReturn_Current=L'\n'; ToReturn_Current++;
        *ToReturn_Current=L'\0'; ToReturn_Current++;

        /* Time stamps */
        handle->pts_in=priv->pts;
        handle->pts_out=pts;

        /* Flushing */
        priv->Number++;
        priv->Hascontent=0;
    }

    /* Getting height (currently, always 15 lines, but this may change in the future) and width (it depends of the display aspect ratio) of the screen */
    while (caption->minimal[MaxLine]!=NULL)
        MaxLine++;
    while (caption->minimal[0][MaxColumn].value!=L'\0')
        MaxColumn++;
    if (MaxLine==0 || MaxColumn==0)
        return NULL; /* Problem */
    
    /* Creating the Caption handle if it is not yet created */
    if (priv->Characters==NULL)
    {
        priv->Characters=(ccdecoder_minimal_character**)calloc(MaxLine+1, sizeof(ccdecoder_minimal_character*));
        for (Line=0; Line<MaxLine; Line++)
            priv->Characters[Line]=(ccdecoder_minimal_character*)calloc(MaxColumn+1, sizeof(ccdecoder_minimal_character)); /* +1 for \0 */
        priv->Characters[MaxLine]=NULL;
    }

    /* Searching top line */
    for (Line=0; caption->minimal[Line] && Y_Min==(size_t)-1; Line++)
        for (Column=0; caption->minimal[Line][Column].value!=L'\0' && Y_Min==(size_t)-1; Column++)
            if (caption->minimal[Line][Column].value!=L' ')
                Y_Min=Line;
            
    /* Searching bottom line */
    if (Y_Min!=(size_t)-1)
        for (Line=MaxLine-1; Y_Max==(size_t)-1; Line--)
            for (Column=0; caption->minimal[Line][Column].value!=L'\0' && Y_Max==(size_t)-1; Column++)
                if (caption->minimal[Line][Column].value!=L' ')
                    Y_Max=Line;

    /* Saving this screen (we need end pts before releasing this screen) */
    if (Y_Min!=(size_t)-1) /* If some content now */
    {
        size_t Output_pos, Output_Max;

        /* Cleaning up */
        for (Output_pos=0; priv->Characters[Output_pos]; Output_pos++)
            memset(priv->Characters[Output_pos], 0x00, (MaxColumn+1)*sizeof(ccdecoder_minimal_character)); /* +1 for \0 */
                
        /* Copying new text */
        Output_Max=0;
        for (Line=Y_Min; Line<=Y_Max; Line++)
        {
            size_t X_Min=(size_t)-1, X_Max=(size_t)-1;
            
            /* Searching left character */
            for (Column=0; caption->minimal[Line][Column].value && X_Min==(size_t)-1; Column++)
                if (caption->minimal[Line][Column].value!=L' ')
                {
                    X_Min=Column;

                    /* Searching right character */
                    for (Column=MaxColumn-1; X_Max==(size_t)-1; Column--)
                        if (caption->minimal[Line][Column].value!=L' ')
                            X_Max=Column;
                }
                    
            if (X_Min!=(size_t)-1) /* If some content */
            {
                for (Column=X_Min; Column<=X_Max; Column++)
                    priv->Characters[Output_Max][Column-X_Min]=caption->minimal[Line][Column];
                Output_Max++;
            }
        }

        priv->MaxLine=Y_Max-Y_Min+1;
        priv->MaxColumn=MaxColumn;
        priv->pts=pts;
        priv->Hascontent=1;
    }
    else
        priv->Hascontent=0;

    #ifdef CCDECODER_DUMP
        fprintf(stderr, "ok\n");
    #endif //CCDECODER_DUMP

    return ToReturn;
}

void ccdecoder_subrip_free (ccdecoder_subrip_handle* handle)
{
    ccdecoder_subrip_handle_priv* priv;
    size_t Line;

    if (handle==NULL)
        return;

    priv=(ccdecoder_subrip_handle_priv*)handle->priv;
    if (priv==NULL)
        return;

    // Private part
    for (Line=0; Line<15; Line++) // TODO: change hard-coded value
        free(priv->Characters[Line]);
    free(priv->Characters);
    free(priv);

    //Common part
    free(handle);
}
