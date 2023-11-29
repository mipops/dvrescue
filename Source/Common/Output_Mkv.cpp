/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

#include "Common/Output_Mkv.h"
#include "Common/Merge.h"

#include <iostream>

using namespace std;

static const unsigned char matroska_writer_header[]                 = { 0x1A, 0x45, 0xDF, 0xA3, 0xA3, 0x42, 0x86, 0x81, 0x01, 0x42, 0xF7, 0x81, 0x01, 0x42, 0xF2, 0x81, 0x04, 0x42, 0xF3, 0x81, 0x08, 0x42, 0x82, 0x88, 0x6D, 0x61, 0x74, 0x72, 0x6F, 0x73, 0x6B, 0x61, 0x42, 0x87, 0x81, 0x04, 0x42, 0x85, 0x81, 0x02, 0x18, 0x53, 0x80, 0x67, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x11, 0x4D, 0x9B, 0x74, 0xC6, 0xEC, 0x84, 0x00, 0x00, 0x00, 0x00, 0x4D, 0xBB, 0x8B, 0x53, 0xAB, 0x84, 0x15, 0x49, 0xA9, 0x66, 0x53, 0xAC, 0x81, 0xA1, 0x4D, 0xBB, 0x8B, 0x53, 0xAB, 0x84, 0x16, 0x54, 0xAE, 0x6B, 0x53, 0xAC, 0x81, 0xEF, 0x4D, 0xBB, 0x8C, 0x53, 0xAB, 0x84, 0x12, 0x54, 0xC3, 0x67, 0x53, 0xAC, 0x82, 0x01, 0xD1, 0xEC, 0x93, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x49, 0xA9, 0x66, 0xC9, 0xEC, 0x84, 0x00, 0x00, 0x00, 0x00, 0x2A, 0xD7, 0xB1, 0x83, 0x0F, 0x42, 0x40, 0x4D, 0x80, 0x8C, 0x64, 0x76, 0x72, 0x65, 0x73, 0x63, 0x75, 0x65, 0x30, 0x2E, 0x30, 0x30, 0x57, 0x41, 0x8C, 0x64, 0x76, 0x72, 0x65, 0x73, 0x63, 0x75, 0x65, 0x30, 0x2E, 0x30, 0x30, 0xEC, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x54, 0xAE, 0x6B, 0x41, 0x14, 0xEC, 0x84, 0x00, 0x00, 0x00, 0x00, 0xAE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0xD7, 0x81, 0x01, 0x73, 0xC5, 0x81, 0x01, 0xEC, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x81, 0x00, 0x22, 0xB5, 0x9C, 0x83, 0x75, 0x6E, 0x64, 0x83, 0x81, 0x01, 0x23, 0xE3, 0x83, 0x84, 0x01, 0xFD, 0x22, 0x8A, 0x86, 0x8F, 0x56, 0x5F, 0x4D, 0x53, 0x2F, 0x56, 0x46, 0x57, 0x2F, 0x46, 0x4F, 0x55, 0x52, 0x43, 0x43, 0xE0, 0xA6, 0xB0, 0x82, 0x00, 0x00, 0xBA, 0x82, 0x00, 0x00, 0x9A, 0x81, 0x01, 0x9D, 0x81, 0x06, 0x54, 0xB0, 0x81, 0x04, 0x54, 0xBA, 0x81, 0x03, 0x54, 0xB2, 0x81, 0x03, 0xEC, 0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0xA2, 0xA8, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x76, 0x32, 0x31, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0xD7, 0x81, 0x02, 0x73, 0xC5, 0x81, 0x02, 0xEC, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x81, 0x00, 0x22, 0xB5, 0x9C, 0x83, 0x75, 0x6E, 0x64, 0x86, 0x8D, 0x41, 0x5F, 0x50, 0x43, 0x4D, 0x2F, 0x49, 0x4E, 0x54, 0x2F, 0x4C, 0x49, 0x54, 0x83, 0x81, 0x02, 0xE1, 0x91, 0x9F, 0x81, 0x02, 0xB5, 0x88, 0x40, 0xE7, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x64, 0x81, 0x20, 0xEC, 0x9D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const unsigned char matroska_writer_header_seek[]            = { 0x4D, 0xBB, 0x92, 0x53, 0xAB, 0x84, 0x1C, 0x53, 0xBB, 0x6B, 0x53, 0xAC, 0x88 };
static const unsigned char matroska_writer_header_timecode[]        = { 0x41, 0xE4, 0x8A, 0x41, 0xE7, 0x83, 0x31, 0x32, 0x31, 0x41, 0xF0, 0x81, 0x79 };
static const unsigned char matroska_writer_header_texttrack[]       = { 0xAE, 0xA1, 0xD7, 0x81, 0x03, 0x73, 0xC5, 0x81, 0x03, 0xEC, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x81, 0x11, 0x86, 0x8B, 0x53, 0x5F, 0x54, 0x45, 0x58, 0x54, 0x2F, 0x55, 0x54, 0x46, 0x38, 0x9C, 0x81, 0x00 };
static const size_t matroska_writer_header_seek_offset              = 0x006A;
static const size_t matroska_writer_header_totalduration_offset     = 0x0118;
static const size_t matroska_writer_header_defaultduration_offset   = 0x015B;
static const size_t matroska_writer_header_width_offset             = 0x0174;
static const size_t matroska_writer_header_height_offset            = 0x0178;
static const size_t matroska_writer_header_sizeimage_offset         = 0x01AF;
static const size_t matroska_writer_header_avi_width_offset         = 0x019F;
static const size_t matroska_writer_header_avi_height_offset        = 0x01A3;
static const size_t matroska_writer_header_timecode_offset          = 0x01C3;
static const size_t matroska_writer_header_texttrack_offset         = 0x021A;

static void store_b1(char*& buffer, int value)
{
    buffer[0] = (unsigned char)(value);
    buffer++;
};

static void store_b2(char*& buffer, int value)
{
    buffer[0] = (unsigned char)(value >> 8);
    buffer[1] = (unsigned char)(value);
    buffer += 2;
};

static void store_b4(char* &buffer, int value)
{
    buffer[0] = (unsigned char)(value >> 24);
    buffer[1] = (unsigned char)(value >> 16);
    buffer[2] = (unsigned char)(value >>  8);
    buffer[3] = (unsigned char)(value);
    buffer += 4;
};

static void store_b8(char*& buffer, long long value)
{
    buffer[0] = (unsigned char)(value >> 56);
    buffer[1] = (unsigned char)(value >> 48);
    buffer[2] = (unsigned char)(value >> 40);
    buffer[3] = (unsigned char)(value >> 32);
    buffer[4] = (unsigned char)(value >> 24);
    buffer[5] = (unsigned char)(value >> 16);
    buffer[6] = (unsigned char)(value >> 8);
    buffer[7] = (unsigned char)(value);
    buffer += 8;
};

static void store_l2(char*& buffer, int value)
{
    buffer[0] = (unsigned char)(value);
    buffer[1] = (unsigned char)(value >> 8);
    buffer += 2;
};

static void store_l4(char* &buffer, int value)
{
    buffer[0] = (unsigned char)(value);
    buffer[1] = (unsigned char)(value >>  8);
    buffer[2] = (unsigned char)(value >> 16);
    buffer[3] = (unsigned char)(value >> 24);
    buffer += 4;
};

static void store_e1(char*& buffer, int value)
{
    store_b1(buffer, value | 0x80);
};

static void store_e4(char* &buffer, int value)
{
    store_b4(buffer, value | 0x10000000);
};

static void store_e8(char*& buffer, long long value)
{
    store_b8(buffer, value | 0x0100000000000000LL);
};

static void store_bf8(char*& buffer, double value)
{
    long long* fake_int;
    fake_int = (long long*)&value;
    store_b8(buffer, *fake_int);
}

matroska_writer::matroska_writer(std::ostream* output, int width, int height, int framerate_num, int framerate_den, bool has_timecode)
    : width(width)
    , height(height)
    , framerate_num(framerate_num)
    , framerate_den(framerate_den)
    , has_timecode(has_timecode)
    , frame_number(0)
    , output_size(sizeof(matroska_writer_header) - 0x34)
    , output(output)
{
    write_header();
}

void matroska_writer::write_header()
{
    char buffer[sizeof(matroska_writer_header)];
    memcpy(buffer, matroska_writer_header, sizeof(matroska_writer_header));
    char* cur;

    if (!cues.empty())
    {
        double timecode_ms = (double)frame_number * 1000 * framerate_den / framerate_num;

        cur = buffer + 0x2C;
        store_e8(cur, output_size);
        cur = buffer + matroska_writer_header_seek_offset;
        memcpy(cur, matroska_writer_header_seek, sizeof(matroska_writer_header_seek));
        cur += sizeof(matroska_writer_header_seek);
        store_b8(cur, output_size - 0x34 - (8 + 27 * cues.size()));
        cur = buffer + matroska_writer_header_totalduration_offset;
        store_b2(cur, 0x4489); // Duration
        store_b1(cur, 0x88);
        store_bf8(cur, timecode_ms);
    }
    cur = buffer + matroska_writer_header_defaultduration_offset;
    store_b4(cur, (unsigned)(((unsigned long long)framerate_den * 1000000000 + framerate_num / 2) / framerate_num));
    cur = buffer + matroska_writer_header_width_offset;
    store_b2(cur, width);
    cur = buffer + matroska_writer_header_height_offset;
    store_b2(cur, height);
    cur = buffer + matroska_writer_header_sizeimage_offset;
    store_b2(cur, width * height * 20 * 32 / (30 * 8));
    cur = buffer + matroska_writer_header_avi_width_offset;
    store_l2(cur, width);
    cur = buffer + matroska_writer_header_avi_height_offset;
    store_l2(cur, height);
    if (has_timecode)
    {
        cur = buffer + matroska_writer_header_timecode_offset;
        memcpy(cur, matroska_writer_header_timecode, sizeof(matroska_writer_header_timecode));
        cur = buffer + matroska_writer_header_texttrack_offset;
        memcpy(cur, matroska_writer_header_texttrack, sizeof(matroska_writer_header_texttrack));
    }

    output->write(buffer, sizeof(matroska_writer_header));
}

void matroska_writer::write_frame(const char* video_buffer, int video_size, const char* audio_buffer, int audio_size, timecode_struct timecode)
{
    double timecode_ms = (double)frame_number * 1000 * framerate_den / framerate_num + 0.5;
    auto timecode_s = timecode_ms / 1000;
    if (timecode_s >= cues.size())
        cues.push_back({ (unsigned long long)timecode_ms, output_size });
    int total_size = 4 + 1 + 1 + (1 + 4 + 1 + 2 + 1) * 2 + video_size + audio_size;
    if (timecode.is_valid())
        total_size += 1 + 4 + 18 + (1 + 4 + 1 + 2 + 1) * 1 + 11;
    char buffer[0x20];
    char* cur;
    size_t size;

    // Video
    cur = buffer;
    store_b4(cur, 0x1F43B675); // Cluster
    store_e4(cur, total_size);
    store_b1(cur, 0xE7); // TimeStamp
    store_b1(cur, 0x84);
    store_b4(cur, timecode_ms);
    if (timecode.is_valid())
    {
        store_b1(cur, 0xA0); // BlockGroup
        store_e4(cur, 1 + 4 + 1 + 2 + 1 + video_size + (timecode.is_valid() ? 18 : 0));
        store_b1(cur, 0xA1); // Block
    }
    else
    {
        store_b1(cur, 0xA3); // SimpleBlock
    }
    store_e4(cur, 1 + 2 + 1 + video_size);
    store_b4(cur, 0x81000080); // TrackNumber + relative TimeStamp + Flags
    size = cur - buffer;
    output->write(buffer, size);
    output_size += size;
    output->write(video_buffer, video_size);
    output_size += video_size;

    cur = buffer;
    if (timecode.is_valid())
    {
        unsigned long long timecode_value = ((unsigned long long)(((timecode.hours   / 10) & 3)) << 56)
                                          | ((unsigned long long)(((timecode.hours   % 10)    )) << 48)
                                          | ((unsigned long long)(((timecode.minutes / 10) & 7)) << 40)
                                          | ((unsigned long long)(((timecode.minutes / 10)    )) << 32)
                                          | ((unsigned long long)(((timecode.seconds / 10) & 7)) << 24)
                                          | ((unsigned long long)(((timecode.seconds / 10)    )) << 16)
                                          | ((unsigned long long)(((timecode.dropframe)       )) << 10)
                                          | ((unsigned long long)(((timecode.frames  / 10) & 3)) <<  8)
                                          | ((unsigned long long)(((timecode.frames  % 10)    ))      );
        store_b2(cur, 0x75A1); // BlockMore
        store_b8(cur, 0x8FA68DEE8179A588LL); // BlockMore size + BlockAddID + BlockAdditional
        store_b8(cur, timecode_value);
    }
    store_b1(cur, 0xA3); // SimpleBlock
    store_e4(cur, 1 + 2 + 1 + audio_size);
    store_b4(cur, 0x82000080); // TrackNumber + relative TimeStamp + Flags
    size = cur - buffer;
    output->write(buffer, size);
    output_size += size;
    output->write(audio_buffer, audio_size);
    output_size += audio_size;

    if (timecode.is_valid())
    {
        char text_buffer[11] = { '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0' };
        text_buffer[ 0] += (timecode.hours   / 10) & 3;
        text_buffer[ 1] +=  timecode.hours   % 10;
        text_buffer[ 3] += (timecode.minutes / 10) & 3;
        text_buffer[ 4] +=  timecode.minutes % 10;
        text_buffer[ 6] += (timecode.seconds / 10) & 3;
        text_buffer[ 7] +=  timecode.seconds % 10;
        if (timecode.dropframe)
            text_buffer[ 8] = ';';
        text_buffer[ 9] += (timecode.frames  / 10) & 3;
        text_buffer[10] +=  timecode.frames  % 10;

        cur = buffer;
        store_b1(cur, 0xA3); // SimpleBlock
        store_e4(cur, 1 + 2 + 1 + 11);
        store_b4(cur, 0x83000000); // TrackNumber + relative TimeStamp + Flags
        size = cur - buffer;
        output->write(buffer, size);
        output_size += size;
        output->write(text_buffer, 11);
        output_size += 11;
    }

    frame_number++;

    cerr << "\33[2K\rCapture frame " << frame_number << ", press " << (InControl ? "q" : "ctrl+c") << " to stop.";
}

void matroska_writer::close(std::ofstream* output)
{
    char buffer[sizeof(matroska_writer_header)];
    char* cur;

    // Cues
    size_t cues_size = 27 * cues.size();
    cur = buffer;
    store_b4(cur, 0x1C53BB6B); // Cues
    store_e4(cur, cues_size);
    cues_size += 8;
    for (const auto& cue : cues)
    {
        store_b4(cur, 0xBB99B388); // CuePoint + CueTime
        store_b8(cur, cue.time);
        store_b4(cur, 0xB78DF781); // CueTrackPosition + CueTrack
        store_b1(cur, 1);
        store_b2(cur, 0xF188); // CueClusterPosition
        store_b8(cur, cue.offset);
        auto size = cur - buffer;
        if (sizeof(matroska_writer_header) - size < 27)
        {
            output->write(buffer, size);
            output_size += size;
            cur = buffer;
        }
    }
    if (cur > buffer)
    {
        auto size = cur - buffer;
        output->write(buffer, size);
        output_size += size;
    }

    output->seekp(0);
    if (!output)
        return;

    write_header();
}
