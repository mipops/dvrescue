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
#include <wchar.h>
#include "ccdecoder_onscreen.h"
#include "ccdecoder.h"

wchar_t* ccdecoder_onscreen(ccdecoder_caption* caption, double pts)
{
    wchar_t*    ToReturn;
    wchar_t*    ToReturn_Current;
    size_t      ToReturn_size=0, MaxColumn=0, Line, Column;

    if (!(caption->status&ccdecoder_haschanged) || !caption->minimal)
        return NULL;
        
    while (caption->minimal[0][MaxColumn].value!=L'\0')
        MaxColumn++;
    
    ToReturn_size=100+(1+MaxColumn+1+1)*(1+15+1); /* 100 for extra data, * + MaxColumn columns max + * + \n */
    ToReturn=ToReturn_Current=(wchar_t*)malloc(ToReturn_size*sizeof(wchar_t));
    ToReturn_Current[0]=L'\0';

    swprintf(ToReturn_Current, ToReturn_size, L"PTS=%f\n", (float)pts); ToReturn_size-=wcslen(ToReturn_Current); ToReturn_Current+=wcslen(ToReturn_Current);
    for (Column=0; Column<MaxColumn+2; Column++)
    {
        swprintf(ToReturn_Current, ToReturn_size, L"*"); ToReturn_size-=wcslen(ToReturn_Current); ToReturn_Current+=wcslen(ToReturn_Current);
    }
    swprintf(ToReturn_Current, ToReturn_size, L"\n"); ToReturn_size-=wcslen(ToReturn_Current); ToReturn_Current+=wcslen(ToReturn_Current);
    for (Line=0; caption->minimal[Line]; Line++)
    {
        swprintf(ToReturn_Current, ToReturn_size, L"*"); ToReturn_size-=wcslen(ToReturn_Current); ToReturn_Current+=wcslen(ToReturn_Current);
        for (Column=0; caption->minimal[Line][Column].value!=L'\0'; Column++)
        {
            swprintf(ToReturn_Current, ToReturn_size, L"%lc", caption->minimal[Line][Column].value); ToReturn_size-=wcslen(ToReturn_Current); ToReturn_Current+=wcslen(ToReturn_Current);
        }
        swprintf(ToReturn_Current, ToReturn_size, L"*\n"); ToReturn_size-=wcslen(ToReturn_Current); ToReturn_Current+=wcslen(ToReturn_Current);
    }
    for (Column=0; Column<MaxColumn+2; Column++)
    {
        swprintf(ToReturn_Current, ToReturn_size, L"*"); ToReturn_size-=wcslen(ToReturn_Current); ToReturn_Current+=wcslen(ToReturn_Current);
    }
    swprintf(ToReturn_Current, ToReturn_size, L"\n\n"); ToReturn_size-=wcslen(ToReturn_Current); ToReturn_Current+=wcslen(ToReturn_Current);

    return ToReturn;
}
