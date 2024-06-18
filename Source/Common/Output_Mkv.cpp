/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

#include "Common/Output_Mkv.h"
#include "Common/Merge.h"
#include "Common/Config.h"

#include <iostream>

using namespace std;

static const unsigned char matroska_writer_header_begin[]           = { 0x1A, 0x45, 0xDF, 0xA3, 0xA3, 0x42, 0x86, 0x81, 0x01, 0x42, 0xF7, 0x81, 0x01, 0x42, 0xF2, 0x81, 0x04, 0x42, 0xF3, 0x81, 0x08, 0x42, 0x82, 0x88, 0x6D, 0x61, 0x74, 0x72, 0x6F, 0x73, 0x6B, 0x61, 0x42, 0x87, 0x81, 0x04, 0x42, 0x85, 0x81, 0x02, 0x18, 0x53, 0x80, 0x67, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x11, 0x4D, 0x9B, 0x74, 0xC6, 0xEC, 0x84, 0x00, 0x00, 0x00, 0x00, 0x4D, 0xBB, 0x8B, 0x53, 0xAB, 0x84, 0x15, 0x49, 0xA9, 0x66, 0x53, 0xAC, 0x81, 0xA1, 0x4D, 0xBB, 0x8B, 0x53, 0xAB, 0x84, 0x16, 0x54, 0xAE, 0x6B, 0x53, 0xAC, 0x81, 0xF3, 0xEC, 0x8D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x93, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x49, 0xA9, 0x66, 0xCD, 0xEC, 0x84, 0x00, 0x00, 0x00, 0x00, 0x2A, 0xD7, 0xB1, 0x83, 0x0F, 0x42, 0x40, 0x4D, 0x80, 0x8E, 0x44, 0x56, 0x52, 0x65, 0x73, 0x63, 0x75, 0x65, 0x20, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x57, 0x41, 0x8E, 0x44, 0x56, 0x52, 0x65, 0x73, 0x63, 0x75, 0x65, 0x20, 0x30, 0x30, 0x2E, 0x30, 0x30, 0xEC, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x54, 0xAE, 0x6B, 0x7F, 0xFF, 0xEC, 0x84, 0x00, 0x00, 0x00, 0x00, 0xAE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD7, 0x81, 0x01, 0x73, 0xC5, 0x81, 0x01, 0xEC, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x81, 0x00, 0x22, 0xB5, 0x9C, 0x83, 0x75, 0x6E, 0x64, 0x83, 0x81, 0x01, 0x23, 0xE3, 0x83, 0x84, 0x01, 0xFD, 0x22, 0x8A, 0x86 };
static const unsigned char matroska_writer_header_seek[]            = { 0x4D, 0xBB, 0x92, 0x53, 0xAB, 0x84, 0x1C, 0x53, 0xBB, 0x6B, 0x53, 0xAC, 0x88 };
static const unsigned char matroska_writer_header_video_id_v210[]   = { 0x8F, 0x56, 0x5F, 0x4D, 0x53, 0x2F, 0x56, 0x46, 0x57, 0x2F, 0x46, 0x4F, 0x55, 0x52, 0x43, 0x43};
static const unsigned char matroska_writer_header_video_id_UYVY[]   = { 0x8E, 0x56, 0x5F, 0x55, 0x4E, 0x43, 0x4F, 0x4D, 0x50, 0x52, 0x45, 0x53, 0x53, 0x45, 0x44 };
static const unsigned char matroska_writer_header_video_middle[]    = { 0xE0, 0xFF, 0xB0, 0x82, 0x00, 0x00, 0xBA, 0x82, 0x00, 0x00, 0x9A, 0x81, 0x01, 0x9D, 0x81, 0x06, 0x54, 0xB0, 0x81, 0x04, 0x54, 0xBA, 0x81, 0x03, 0x54, 0xB2, 0x81, 0x03, 0x55, 0xB0, 0x84, 0x55, 0xB9, 0x81, 0x01 };
static const unsigned char matroska_writer_header_video_mid_UYVY[]  = { 0x2E, 0xB5, 0x24, 0x84, 0x55, 0x59, 0x56, 0x59 }; 
static const unsigned char matroska_writer_header_video_priv_v210[] = { 0x63, 0xA2, 0xA8, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x76, 0x32, 0x31, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const unsigned char matroska_writer_header_timecode_void[]   = { 0xEC, 0x8B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const unsigned char matroska_writer_header_timecode[]        = { 0x41, 0xE4, 0x8A, 0x41, 0xE7, 0x83, 0x31, 0x32, 0x31, 0x41, 0xF0, 0x81, 0x79 };
static const unsigned char matroska_writer_header_audio[]           = { 0xAE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0xD7, 0x81, 0x02, 0x73, 0xC5, 0x81, 0x02, 0xEC, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x81, 0x00, 0x22, 0xB5, 0x9C, 0x83, 0x75, 0x6E, 0x64, 0x86, 0x8D, 0x41, 0x5F, 0x50, 0x43, 0x4D, 0x2F, 0x49, 0x4E, 0x54, 0x2F, 0x4C, 0x49, 0x54, 0x83, 0x81, 0x02, 0xE1, 0x91, 0x9F, 0x81, 0x02, 0xB5, 0x88, 0x40, 0xE7, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x64, 0x81, 0x20 };
static const unsigned char matroska_writer_header_texttrack_void[]  = { 0xEC, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const unsigned char matroska_writer_header_texttrack[]       = { 0xAE, 0xA1, 0xD7, 0x81, 0x03, 0x73, 0xC5, 0x81, 0x03, 0xEC, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x81, 0x11, 0x86, 0x8B, 0x53, 0x5F, 0x54, 0x45, 0x58, 0x54, 0x2F, 0x55, 0x54, 0x46, 0x38, 0x9C, 0x81, 0x00 };
static const size_t matroska_writer_header_size_offset              = 0x002C;
static const size_t matroska_writer_header_seek_crc_beg             = 0x0039;
static const size_t matroska_writer_header_seek_offset              = 0x006A;
static const size_t matroska_writer_header_seek_crc_end             = 0x007F;
static const size_t matroska_writer_header_info_crc_beg             = 0x00DA;
static const size_t matroska_writer_header_muxing_offset            = 0x00F3;
static const size_t matroska_writer_header_writing_offset           = 0x0104;
static const size_t matroska_writer_header_totalduration_offset     = 0x011C;
static const size_t matroska_writer_header_info_crc_end             = 0x0127;
static const size_t matroska_writer_header_tracks_crc_beg           = 0x012D;
static const size_t matroska_writer_header_defaultduration_offset   = 0x015F;
static const size_t matroska_writer_header_width_offset             = 0x0004;
static const size_t matroska_writer_header_height_offset            = 0x0008;
static const size_t matroska_writer_header_v210_avi_width_offset    = 0x0007;
static const size_t matroska_writer_header_v210_avi_height_offset   = 0x000B;
static const size_t matroska_writer_header_avi_sizeimage_offset     = 0x0017;

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

static void store_e2(char*& buffer, int value)
{
    store_b2(buffer, value | 0x4000);
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

//---------------------------------------------------------------------------
// CRC_32_Table (Little Endian bitstream)
// The CRC in use is the IEEE-CRC-32 algorithm as used in the ISO 3309 standard and in section 8.1.1.6.2 of ITU-T recommendation V.42, with initial value of 0xFFFFFFFF. The CRC value MUST be computed on a little endian bitstream and MUST use little endian storage.
// A CRC is computed like this:
// Init: unsigned crc32 ^= 0;
// for each data byte do
//     crc32=(crc32>>8) ^ matroska_crc32_table[(crc32&0xFF)^*Current++];
// End: crc32 ^= 0;
static const unsigned matroska_crc32_table[256] =
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02Ef8D,
};

static unsigned matroska_crc32_compute(const char* buffer, size_t size)
{
    unsigned crc32 = 0xFFFFFFFF;
    auto cur = (unsigned char*)buffer;
    auto end = cur + size;
    while (cur < end)
        crc32 = (crc32 >> 8) ^ matroska_crc32_table[(crc32 & 0xFF) ^ *cur++];
    crc32 ^= 0xFFFFFFFF;
    return crc32;
}

static void matroska_crc32_store(char* buffer, size_t size)
{
    if (size < 6)
        return;
    auto crc32 = matroska_crc32_compute(buffer + 6, size - 6);
    store_b1(buffer, 0xBF); // CRC-32
    store_b1(buffer, 0x84);
    store_l4(buffer, crc32);
}

matroska_writer::matroska_writer(std::ostream* output, video_format vid_fmt, int width, int height, int framerate_num, int framerate_den, bool has_timecode)
    : vid_fmt(vid_fmt)
    , width(width)
    , height(height)
    , framerate_num(framerate_num)
    , framerate_den(framerate_den)
    , has_timecode(has_timecode)
    , buffer(nullptr)
    , buffer_size(0)
    , frame_number(0)
    , segment_size(0)
    , output(output)
{
    write_header();
}

matroska_writer::~matroska_writer()
{
    delete[] buffer;
}

void matroska_writer::write_header()
{
    char buffer[64 * 1024];
    memcpy(buffer, matroska_writer_header_begin, sizeof(matroska_writer_header_begin));
    auto offset = sizeof(matroska_writer_header_begin);
    char* cur;

    // Version
    if (strlen(Program_Version) == 5)
    {
        cur = buffer + matroska_writer_header_muxing_offset;
        memcpy(cur, Program_Version, 5);
        cur = buffer + matroska_writer_header_writing_offset;
        memcpy(cur, Program_Version, 5);
    }

    // Total size + Cues + Duration
    if (!cues.empty())
    {
        double timecode_ms = (double)frame_number * 1000 * framerate_den / framerate_num;

        cur = buffer + matroska_writer_header_size_offset;
        store_e8(cur, segment_size);
        
        cur = buffer + matroska_writer_header_seek_offset;
        memcpy(cur, matroska_writer_header_seek, sizeof(matroska_writer_header_seek));
        cur += sizeof(matroska_writer_header_seek);
        store_b8(cur, segment_size - (8 + 6 + 27 * cues.size()));

        cur = buffer + matroska_writer_header_totalduration_offset;
        store_b2(cur, 0x4489); // Duration
        store_b1(cur, 0x88);
        store_bf8(cur, timecode_ms);
    }
    
    // Video track
    cur = buffer + matroska_writer_header_defaultduration_offset;
    store_b4(cur, (unsigned)(((unsigned long long)framerate_den * 1000000000 + framerate_num / 2) / framerate_num));
    switch (vid_fmt)
    {
    case v210:
        memcpy(buffer + offset, matroska_writer_header_video_id_v210, sizeof(matroska_writer_header_video_id_v210));
        offset += sizeof(matroska_writer_header_video_id_v210);
        break;
    case UYVY:
        memcpy(buffer + offset, matroska_writer_header_video_id_UYVY, sizeof(matroska_writer_header_video_id_UYVY));
        offset += sizeof(matroska_writer_header_video_id_UYVY);
        break;
    }
    memcpy(buffer + offset, matroska_writer_header_video_middle, sizeof(matroska_writer_header_video_middle));
    auto track_video_begin_pos = offset + matroska_writer_header_width_offset - 2;
    cur = buffer + offset + matroska_writer_header_width_offset;
    store_b2(cur, width);
    cur = buffer + offset + matroska_writer_header_height_offset;
    store_b2(cur, height);
    offset += sizeof(matroska_writer_header_video_middle);
    switch (vid_fmt)
    {
    case UYVY:
        memcpy(buffer + offset, matroska_writer_header_video_mid_UYVY, sizeof(matroska_writer_header_video_mid_UYVY));
        offset += sizeof(matroska_writer_header_video_mid_UYVY);
        break;
    default:;
    }
    cur = buffer + track_video_begin_pos - 1;
    store_e1(cur, (uint8_t)(offset - track_video_begin_pos)); // video element size
    switch (vid_fmt)
    {
    case v210:
        memcpy(buffer + offset, matroska_writer_header_video_priv_v210, sizeof(matroska_writer_header_video_priv_v210));
        cur = buffer + offset + matroska_writer_header_v210_avi_width_offset;
        store_l2(cur, width);
        cur = buffer + offset + matroska_writer_header_v210_avi_height_offset;
        store_l2(cur, height);
        cur = buffer + offset + matroska_writer_header_avi_sizeimage_offset;
        store_l4(cur, width * height * 20 * 32 / (30 * 8));
        offset += sizeof(matroska_writer_header_video_priv_v210);
        break;
    default: ;
    }
    memcpy(buffer + offset, has_timecode ? matroska_writer_header_timecode : matroska_writer_header_timecode_void, sizeof(matroska_writer_header_timecode));
    offset += sizeof(matroska_writer_header_timecode);
    cur = buffer + matroska_writer_header_tracks_crc_beg + 0xF - 1;
    store_b1(cur, (uint8_t)(offset - (matroska_writer_header_tracks_crc_beg + 0xF))); // track element size

    // Audio track
    memcpy(buffer + offset, matroska_writer_header_audio, sizeof(matroska_writer_header_audio));
    offset += sizeof(matroska_writer_header_audio);

    // Timecode track
    memcpy(buffer + offset, has_timecode ? matroska_writer_header_texttrack : matroska_writer_header_texttrack_void, sizeof(matroska_writer_header_texttrack));
    offset += sizeof(matroska_writer_header_texttrack);

    matroska_crc32_store(buffer + matroska_writer_header_seek_crc_beg, matroska_writer_header_seek_crc_end - matroska_writer_header_seek_crc_beg);
    matroska_crc32_store(buffer + matroska_writer_header_info_crc_beg, matroska_writer_header_info_crc_end - matroska_writer_header_info_crc_beg);
    cur = buffer + matroska_writer_header_tracks_crc_beg - 2;
    store_e2(cur, (uint16_t)(offset - matroska_writer_header_tracks_crc_beg)); // tracks element size
    matroska_crc32_store(buffer + matroska_writer_header_tracks_crc_beg, offset - matroska_writer_header_tracks_crc_beg);
    output->write(buffer, offset);
    output->flush();
    segment_size = offset - 0x34;
}

void matroska_writer::write_frame(const char* video_buffer, int video_size, const char* audio_buffer, int audio_size, TimeCode timecode)
{
    // Cues
    auto timecode_ms = (frame_number * 1000 * framerate_den + framerate_num / 2 - 1) / framerate_num;
    auto timecode_s = timecode_ms / 1000;
    if (timecode_s >= cues.size())
        cues.push_back({ (unsigned long long)timecode_ms, segment_size });
    
    // Buffer
    int total_size = 6 + 4 + 1 + 1 + (1 + 4 + 1 + 2 + 1) * 2 + video_size + audio_size;
    if (timecode.HasValue())
        total_size += 1 + 4 + 18 + (1 + 4 + 1 + 2 + 1) * 1 + 11;
    auto total_size_with_header = 8 + total_size;
    if (buffer_size < total_size_with_header)
    {
        buffer_size = total_size_with_header;
        delete[] buffer;
        buffer = new char[buffer_size];
    }
    auto cur = buffer;

    // Header
    store_b4(cur, 0x1F43B675); // Cluster
    store_e4(cur, total_size);
    cur += 6; // CRC-32
    store_b1(cur, 0xE7); // TimeStamp
    store_b1(cur, 0x84);
    store_b4(cur, (int)(timecode_ms+0.5));

    // Video
    if (timecode.HasValue())
    {
        store_b1(cur, 0xA0); // BlockGroup
        store_e4(cur, 1 + 4 + 1 + 2 + 1 + video_size + (timecode.HasValue() ? 18 : 0));
        store_b1(cur, 0xA1); // Block
    }
    else
    {
        store_b1(cur, 0xA3); // SimpleBlock
    }
    store_e4(cur, 1 + 2 + 1 + video_size);
    store_b4(cur, 0x81000080); // TrackNumber + relative TimeStamp + Flags
    memcpy(cur, video_buffer, video_size);
    cur += video_size;

    if (timecode.HasValue())
    {
        unsigned long long timecode_value = ((unsigned long long)(((timecode.Hours   / 10) & 3)) << 56)
                                          | ((unsigned long long)(((timecode.Hours   % 10)    )) << 48)
                                          | ((unsigned long long)(((timecode.Minutes / 10) & 7)) << 40)
                                          | ((unsigned long long)(((timecode.Minutes / 10)    )) << 32)
                                          | ((unsigned long long)(((timecode.Seconds / 10) & 7)) << 24)
                                          | ((unsigned long long)(((timecode.Seconds / 10)    )) << 16)
                                          | ((unsigned long long)(((timecode.DropFrame)       )) << 10)
                                          | ((unsigned long long)(((timecode.Frames  / 10) & 3)) <<  8)
                                          | ((unsigned long long)(((timecode.Frames  % 10)    ))      );
        store_b2(cur, 0x75A1); // BlockMore
        store_b8(cur, 0x8FA68DEE8179A588LL); // BlockMore size + BlockAddID + BlockAdditional
        store_b8(cur, timecode_value);
        if (!has_timecode)
            has_timecode=true; // timecode track header will be written when header is written again with cues
    }
    
    // Audio
    store_b1(cur, 0xA3); // SimpleBlock
    store_e4(cur, 1 + 2 + 1 + audio_size);
    store_b4(cur, 0x82000080); // TrackNumber + relative TimeStamp + Flags
    memcpy(cur, audio_buffer, audio_size);
    cur += audio_size;

    if (timecode.HasValue())
    {
        char text_buffer[11] = { '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0' };
        text_buffer[ 0] += (timecode.Hours   / 10) & 3;
        text_buffer[ 1] +=  timecode.Hours   % 10;
        text_buffer[ 3] += (timecode.Minutes / 10) & 7;
        text_buffer[ 4] +=  timecode.Minutes % 10;
        text_buffer[ 6] += (timecode.Seconds / 10) & 7;
        text_buffer[ 7] +=  timecode.Seconds % 10;
        if (timecode.DropFrame)
            text_buffer[ 8] = ';';
        text_buffer[ 9] += (timecode.Frames  / 10) & 3;
        text_buffer[10] +=  timecode.Frames  % 10;

        store_b1(cur, 0xA3); // SimpleBlock
        store_e4(cur, 1 + 2 + 1 + 11);
        store_b4(cur, 0x83000000); // TrackNumber + relative TimeStamp + Flags
        memcpy(cur, text_buffer, 11);
        cur += 11;
    }

    auto size = cur - buffer;
    matroska_crc32_store(buffer + 8, total_size);
    output->write(buffer, size);
    segment_size += size;
    frame_number++;
}

void matroska_writer::close(std::ofstream* output)
{
    auto cues_size = 6 + 27 * cues.size();
    if (buffer_size < cues_size)
    {
        buffer_size = cues_size;
        delete[] buffer;
        buffer = new char[buffer_size];
    }
    auto cur = buffer;

    // Header
    store_b4(cur, 0x1C53BB6B); // Cues
    store_e4(cur, (uint32_t)cues_size);
    cur += 6; // CRC-32

    // Cues
    for (const auto& cue : cues)
    {
        store_b4(cur, 0xBB99B388); // CuePoint + CueTime
        store_b8(cur, cue.time);
        store_b4(cur, 0xB78DF781); // CueTrackPosition + CueTrack
        store_b1(cur, 1);
        store_b2(cur, 0xF188); // CueClusterPosition
        store_b8(cur, cue.offset);
    }

    auto size = cur - buffer;
    matroska_crc32_store(buffer + 8, cues_size);
    output->write(buffer, size);
    segment_size += size;

    output->seekp(0);
    if (output)
        write_header();
    output->close();
}
