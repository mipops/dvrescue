/*
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

#ifndef ccdecoder_line21H
#define ccdecoder_line21H

#include "ccdecoder.h"

#ifdef __cplusplus
extern "C" 
{
#endif //__cplusplus

/****************************************************************************/
/* Functions                                                                */
/****************************************************************************/

/**
 * @brief Allocate a new handle
 * @returns 0 if an error has occured, else an handle to be provided to other funtions
 * @note Handle must be freed by ccdecoder_line21_free() or ccdecoder_free()
 */
extern ccdecoder_handle*    ccdecoder_line21_alloc ();

/**
 * @brief Parse a frame
 * @param handle Handle created by ccdecoder_line21_alloc()
 * @param data Pointer to the data to be parsed
 * @param size size (in bytes) of data
 * @param options Options (see ccdecoder_options())
 * @param pts Presentation time stamp (in seconds) (optional, put DBL_MAX if unknown)
 * @param dts Decoding time stamp (in seconds) (optional, put DBL_MAX if unknown)
 * @param dur Duration (in seconds) (optional, put DBL_MAX if unknown)
 * @returns Summary of the status of all parsers (see ccdecoder_status())
 */
extern size_t               ccdecoder_line21_parse (ccdecoder_handle* handle, uint8_t* data, size_t size, size_t options, double pts, double dts, double dur);

/**
 * @brief Free the allocated handle
 * @param handle Handle created by ccdecoder_line21_alloc()
 */
extern void                 ccdecoder_line21_free  (ccdecoder_handle* handle);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* ccdecoder_line21H */
