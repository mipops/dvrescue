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

#ifndef ccdecoder_subripH
#define ccdecoder_subripH

#include "ccdecoder.h"

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

/****************************************************************************/
/* Structures                                                               */
/****************************************************************************/

typedef struct ccdecoder_subrip_handle
{
    void*                           priv;           ///< priv (internal usage), do not use it
    double                          pts_in;
    double                          pts_out;
} ccdecoder_subrip_handle;

/****************************************************************************/
/* Functions                                                                */
/****************************************************************************/

/**
 * @brief Allocate a new handle
 * @returns 0 if an error has occured, else an handle to be provided to other funtions
 * @note Handle must be freed by ccdecoder_subrip_free()
 */
extern ccdecoder_subrip_handle*     ccdecoder_subrip_alloc  ();

/**
 * @brief Provides the text corresponding to a subrip caption element
 * @param handle Handle created by ccdecoder_subrip_alloc()
 * @param caption Handle of the caption (e.g. created with ccdecoder_alloc())
 * @param pts Presentation Time Stamp of the video frame, in seconds
 * @returns text to be added to the subrip file
 */
extern wchar_t*                     ccdecoder_subrip_parse  (ccdecoder_subrip_handle* handle, ccdecoder_caption* caption, double pts); // return value must be freed by the user

/**
 * @brief Free the allocated handle
 * @param handle Handle created by ccdecoder_subrip_alloc()
 */
extern void                         ccdecoder_subrip_free   (ccdecoder_subrip_handle* handle);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* ccdecoder_subripH */
