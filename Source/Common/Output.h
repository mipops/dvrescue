/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include "ProcessFile.h"
#include <string>
#include <set>
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

struct frame_seqn
{
public:
    frame_seqn(const MediaInfo_Event_DvDif_Analysis_Frame_1& Frame) : _Value(Frame.Arb), _Value2(Frame.MoreFlags) {}
    frame_seqn(const MediaInfo_Event_DvDif_Analysis_Frame_1* Frame) : _Value(Frame->Arb), _Value2(Frame->MoreFlags) {}
    inline int Value() { return _Value & 0xF; }                                    //  0- 3
    inline bool HasValue() { return _Value & (1 << 4); }                           //  4
    inline bool NonConsecutive() { return _Value & (1 << 6); }                     //  6
    inline bool Repeat() { return _Value & (1 << 7); }                             //  7
    inline int NonConsecutive_IsLess() { return _Value2 & (1 << 2); }              //  

private:
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::Arb) _Value;
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::MoreFlags) _Value2;
};

struct abst_bf
{
public:
    abst_bf() {}
    abst_bf(decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::AbstBf) Value) : _Value(Value) {}
    inline int BlankFlag() { return _Value & 1; }                                  //  0
    inline int AbsoluteTrackNumber()
    {
        int Temp = (_Value >> 1) & 0x7FFFFF;                                       //  1-23
        if (Temp >= 0x7F8000)
            Temp -= 0x800000;
        return Temp;
    }
    inline bool HasBlankFlagValue() { return (_Value >> 24) & 1; }                 // 24
    inline bool HasAbsoluteTrackNumberValue() { return (_Value >> 25) & 1; }       // 24
    inline bool NonConsecutive() { return (_Value >> 30) & 1; }                    // 30
    inline bool Repeat() { return (_Value >> 31) & 1; }                            // 31

private:
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::AbstBf) _Value;
};

struct timecode
{
public:
    timecode() {}
    timecode(const MediaInfo_Event_DvDif_Analysis_Frame_1& Frame) : _Value(Frame.TimeCode), _Value2(Frame.MoreFlags) {}
    timecode(const MediaInfo_Event_DvDif_Analysis_Frame_1* Frame) : _Value(Frame->TimeCode), _Value2(Frame->MoreFlags) {}
    inline bool HasValue() { return ((_Value >> 8) & 0x1FFFF) != 0x1FFFF; }
    inline int Frames() { return _Value & 0x3F; }                                  //  0- 6
    inline bool DropFrame() { return _Value & (1 << 7); }                          //  7
    inline int TimeInSeconds() { return (_Value >> 8) & 0x1FFFF; }                 //  8-24
    inline bool NonConsecutive() { return _Value & (1 << 30); }                    // 30
    inline bool Repeat() { return _Value & (1 << 31); }                            // 31
    inline int NonConsecutive_IsLess() { return _Value2 & (1 << 1); }              //  

private:
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::TimeCode) _Value;
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::MoreFlags) _Value2;
};

struct rec_date_time
{
public:
    rec_date_time() {}
    rec_date_time(const MediaInfo_Event_DvDif_Analysis_Frame_1& Frame) : _Value1(Frame.RecordedDateTime1), _Value2(Frame.RecordedDateTime2), _Value3(Frame.MoreFlags) {}
    rec_date_time(const MediaInfo_Event_DvDif_Analysis_Frame_1* Frame) : _Value1(Frame->RecordedDateTime1), _Value2(Frame->RecordedDateTime2), _Value3(Frame->MoreFlags) {}
    inline bool HasDate() { return ((_Value1 >> 17) & 0x7F) != 0x7F; }
    inline bool HasTime() { return (_Value1 & 0x1FFFF) != 0x1FFFF; }
    inline int TimeInSeconds() { return _Value1 & 0x1FFFF; }                        // 1  0-16
    inline int Years() { return (_Value1 >> 17) & 0x7F; }                           // 1 17-24
    inline bool End() { return _Value1 & (1 << 28); }                               // 1 28
    inline bool Start() { return _Value1 & (1 << 29); }                             // 1 29
    inline bool NonConsecutive() { return _Value1 & (1 << 30); }                    // 1 30
    inline bool Repeat() { return _Value1 & (1 << 31); }                            // 1 31
    inline int Frames() { return _Value2 & 0x3F; }                                  // 2  0- 5
    inline int Days() { return (_Value2 >> 6) & 0x1F; }                             // 2  6-10
    inline int Months() { return (_Value2 >> 12) & 0x0F; }                          // 2 12-15
    inline int NonConsecutive_IsLess() { return _Value3 & (1 << 0); }               // 3  0

private:
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::RecordedDateTime1) _Value1;
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::RecordedDateTime2) _Value2;
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::MoreFlags) _Value3;
};

struct coherency_flags
{
public:
    coherency_flags(decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::Coherency_Flags) Value) : _Value(Value) {}
    inline bool no_pack_sub() { return ((_Value >> 0) & 0x1); }                     // 0
    inline bool no_pack_vid() { return ((_Value >> 1) & 0x1); }                     // 1
    inline bool no_pack_aud() { return ((_Value >> 2) & 0x1); }                     // 2
    inline bool full_conceal_vid() { return ((_Value >> 3) & 0x1); }                // 3
    inline bool full_conceal_aud() { return ((_Value >> 4) & 0x1); }                // 4
    inline bool no_sourceorcontrol_vid() { return ((_Value >> 5) & 0x1); }          // 5
    inline bool no_sourceorcontrol_aud() { return ((_Value >> 6) & 0x1); }          // 6
    inline bool no_pack() { return no_pack_sub() && no_pack_vid() && no_pack_aud() ; }
    inline bool full_conceal() { return full_conceal_vid() && full_conceal_aud(); }

private:
    decltype(MediaInfo_Event_DvDif_Analysis_Frame_1::Coherency_Flags) _Value;
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
void seconds_to_timestamp(string& Data, double Seconds_Float, int CountAfterComma = 3, bool trunc = false);
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
        vector<uint16_t> Audio_Errors_Values;
    };
    dseq PerDseq;
};

//***************************************************************************
// Writing
//***************************************************************************

return_value Write(ostream& Out, string& ToWrite, ostream* Err = nullptr, const char* const OutName = nullptr, size_t ToWrite_Size = 0);
inline return_value WriteIfBig(ostream& Out, string& ToWrite, ostream* Err = nullptr, const char* const OutName = nullptr)
{
    static const size_t BlockSize = 0x10000;
    if (ToWrite.size() < BlockSize)
        return ReturnValue_OK;
    return Write(Out, ToWrite, Err, OutName, BlockSize);
}
