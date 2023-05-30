/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once

#include <fstream>
#include <vector>

//***************************************************************************
// TimeCode
//***************************************************************************
struct timecode_struct
{
    timecode_struct()
    {
        memset(this, 0xFF, sizeof(*this));
    }

    timecode_struct(unsigned char hours, unsigned char minutes, unsigned char seconds, unsigned char frames, unsigned char frames_max = (unsigned char)-1, bool dropframe = false)
        : hours(hours)
        , minutes(minutes)
        , seconds(seconds)
        , frames(frames)
        , frames_max(frames_max)
        , dropframe(dropframe)
    {
    }

    unsigned char hours;
    unsigned char minutes;
    unsigned char seconds;
    unsigned char frames;
    unsigned char frames_max;
    bool dropframe;

    bool is_valid()
    {
        return hours != (unsigned char)-1;
    }

    timecode_struct& operator++()
    {
        if (frames >= frames_max)
        {
            seconds++;
            frames = 0;
        }
        else
            frames++;
        return *this;
    }
};

//***************************************************************************
// Class matroska_writer
//***************************************************************************

class matroska_writer
{
public:
    matroska_writer(std::ostream* output, int width, int height, int framerate_num, int framerate_den, bool has_timecode = false);

    void write_frame(const char* video_buffer, int video_size, const char* audio_buffer, int audio_size, timecode_struct timecode = timecode_struct());

    void close(std::ofstream* output);

private:
    int width;
    int height;
    int framerate_num;
    int framerate_den;
    bool has_timecode;
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
