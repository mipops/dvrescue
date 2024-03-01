/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once

#include <fstream>
#include <vector>
#include <cstring>

#include "ThirdParty/TimeCode/TimeCode.h"

//***************************************************************************
// Class matroska_writer
//***************************************************************************

class matroska_writer
{
public:
    matroska_writer(std::ostream* output, int width, int height, int framerate_num, int framerate_den, bool has_timecode = false);
    ~matroska_writer();

    void write_frame(const char* video_buffer, int video_size, const char* audio_buffer, int audio_size, TimeCode timecode = TimeCode());

    void close(std::ofstream* output);

private:
    int width;
    int height;
    int framerate_num;
    int framerate_den;
    bool has_timecode;
    char* buffer;
    size_t buffer_size;
    unsigned long long frame_number;
    unsigned long long output_size;
    std::ostream* output;
    struct cue_struct
    {
        unsigned long long time;
        unsigned long long offset;
    };
    typedef std::vector<cue_struct> cues_struct;
    cues_struct cues;

    void write_header();
};
