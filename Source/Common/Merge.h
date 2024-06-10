/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include <string>
#include <vector>
#include "TimeCode.h"
using namespace std;
//---------------------------------------------------------------------------

//***************************************************************************
//
//***************************************************************************

extern vector<string> Merge_InputFileNames;
extern vector<FILE*> Merge_Out;
extern ostream* MergeInfo_Out;
extern vector<string> Merge_OutputFileNames;
extern bool Merge_OutputFileNames_IncludesStdOut;
#ifdef ENABLE_SONY9PIN
extern const char* Control_Port;
#endif
#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
extern uint8_t DeckLinkVideoMode;
extern uint8_t DeckLinkVideoSource;
extern uint8_t DeckLinkAudioSource;
extern uint8_t DeckLinkTimecodeFormat;
#endif
extern size_t Merge_Rewind_Count;
extern uint8_t MergeInfo_Format;
extern uint8_t Verbosity;
extern uint64_t Timeout;
extern uint8_t UseAbst;
extern vector<bool> OutputFrames_Speeds;
extern vector<bool> OutputFrames_Concealeds;
extern bool OutputFrames_NoData;
extern int ShowFrames_Missing;
extern int ShowFrames_Intermediate;
extern bool InControl;
extern bool Foreground;
extern string Device;
extern char Device_Command;
extern bool Device_ForceCapture;
extern unsigned int Device_Mode;
extern float Device_Speed;
struct MediaInfo_Event_DvDif_Analysis_Frame_1;
struct MediaInfo_Event_Global_Demux_4;

//---------------------------------------------------------------------------
class dv_merge
{
public:
    void AddFrameAnalysis(size_t Merge_FilePos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData, float Speed);
    void AddFrameData(size_t Merge_FilePos, const uint8_t* Buffer, size_t Buffer_Size);
    void Finish();

    TimeCode RewindToTimeCode;
};
