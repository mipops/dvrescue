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

#ifndef ccdecoder_onscreenH
#define ccdecoder_onscreenH

#include "ccdecoder.h"

#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus

/****************************************************************************/
/* Functions                                                                */
/****************************************************************************/

/**
 * @brief Provides an human readable view of the caption
 * @param caption Handle of the caption (e.g. created with ccdecoder_alloc())
 * @param pts Presentation Time Stamp of the video frame, in seconds
 * @returns text to be displayed
 */
extern wchar_t*     ccdecoder_onscreen  (ccdecoder_caption* caption, double pts);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* ccdecoder_onscreenH */
