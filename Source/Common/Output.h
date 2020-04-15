/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include "ProcessFile.h"
#include <string>
using namespace std;
//---------------------------------------------------------------------------

//***************************************************************************
// Sizes
//***************************************************************************

const int Sta_Bits = 4;
const int Dseq_Bits = 4;
const int Sta_Size = 1 << Sta_Bits;
const int Dseq_Size = 1 << Dseq_Bits;
const int DseqSta_Size = Dseq_Size * Sta_Size;

//***************************************************************************
// DvDif_Analysis_Frame helpers
//***************************************************************************

struct frame_arb
{
public:
    frame_arb(decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::Arb) Value) : _Value(Value) {}
    inline int Value() { return _Value & 0xF; }
    inline bool HasValue() { return _Value & (1 << 4); }
    inline bool Repeat() { return _Value & (1 << 5); }
    inline bool NonConsecutive() { return _Value & (1 << 6); }

private:
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::Arb) _Value;
};

struct timecode
{
public:
    timecode(decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::TimeCode) Value) : _Value(Value) {}
    inline bool HasValue() { return ((_Value >> 8) & 0x1FFFF) != 0x1FFFF; }
    inline int Frames() { return _Value & 0x3F; }                                  //  0- 6
    inline bool DropFrame() { return _Value & (1 << 7); }                          //  7
    inline int TimeInSeconds() { return (_Value >> 8) & 0x1FFFF; }                 //  8-24
    inline bool Repeat() { return _Value & (1 << 30); }                            // 30
    inline bool NonConsecutive() { return _Value & (1 << 31); }                    // 31

private:
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::TimeCode) _Value;
};

struct rec_date_time
{
public:
    rec_date_time(decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::RecordedDateTime1) Value1, decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::RecordedDateTime2) Value2) : _Value1(Value1), _Value2(Value2) {}
    inline bool HasDate() { return ((_Value1 >> 17) & 0x7F) != 0x7F; }
    inline bool HasTime() { return (_Value1 & 0x1FFFF) != 0x1FFFF; }
    inline int TimeInSeconds() { return _Value1 & 0x1FFFF; }                        // 1  0-16
    inline int Years() { return (_Value1 >> 17) & 0x7F; }                           // 1 17-24
    inline bool End() { return _Value1 & (1 << 28); }                               // 1 28
    inline bool Start() { return _Value1 & (1 << 29); }                             // 1 29
    inline bool Repeat() { return _Value1 & (1 << 30); }                            // 1 30
    inline bool NonConsecutive() { return _Value1 & (1 << 31); }                    // 1 31
    inline int Frames() { return _Value2 & 0x7F; }                                  // 2  0- 7
    inline int Days() { return (_Value2 >> 8) & 0x1F; }                             // 2  8-12
    inline int Months() { return (_Value2 >> 12) & 0x0F; }                          // 2 12-15

private:
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::RecordedDateTime1) _Value1;
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::RecordedDateTime2) _Value2;
};

//***************************************************************************
// Arrays
//***************************************************************************

const size_t chroma_subsampling_size = 3;
extern const char* const chroma_subsampling[chroma_subsampling_size];

//***************************************************************************
// Formating helpers
//***************************************************************************

void timecode_to_string(string& Data, int Seconds, bool DropFrame, int Frames);
void seconds_to_timestamp(string& Data, double Seconds_Float, int CountAfterComma = 3);
void date_to_string(string& Data, int Years, int Months, int Days);
char uint4_to_hex4(int Value);

//***************************************************************************
// Status of a frame
//***************************************************************************

bool Frame_HasErrors(const MediaInfo_Event_DvDif_Analysis_Frame_1& Frame);
inline bool Frame_HasErrors(const MediaInfo_Event_DvDif_Analysis_Frame_1* Frame) { return Frame_HasErrors(*Frame); }

class computed_errors
{
public:
    computed_errors();
    bool Compute(const MediaInfo_Event_DvDif_Analysis_Frame_1& Frame); // All Dseq
    bool Compute(const MediaInfo_Event_DvDif_Analysis_Frame_1& Frame, int Dseq); // Per Dseq

    // Global Data
    size_t Video_Sta_TotalPerSta[Sta_Size];
    size_t Video_Sta_EvenTotalPerSta[Sta_Size];
    size_t Audio_Data_Total;
    size_t Audio_Data_EvenTotal;

    // Per Dseq
    struct dseq
    {
        size_t Video_Sta_TotalPerSta[Sta_Size];
        size_t Video_Sta_EvenTotalPerSta[Sta_Size];
        size_t Audio_Data_Total;
        size_t Audio_Data_EvenTotal;
    };
    dseq PerDseq;
};

//***************************************************************************
// Writing
//***************************************************************************

return_value Write(ostream& Out, string& ToWrite, ostream* Err = nullptr, const char* const OutName = nullptr);
