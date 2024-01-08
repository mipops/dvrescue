/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/ProcessFile.h"
#include "ZenLib/Ztring.h"
#include "Output.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <mutex>
#include <iomanip>
#include <map>
#include <bitset>
#include <set>
#include "TimeCode.h"
#include "CLI/CLI_Help.h"
using namespace ZenLib;
uint64_t VariableSize(const uint8_t* Buffer, size_t& Buffer_Offset, size_t Buffer_Size);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
vector<string> Merge_InputFileNames;
vector<FILE*> Merge_Out;
uint64_t Merge_Out_Size = -1;
vector<string> Merge_OutputFileNames;
bool Merge_OutputFileNames_IncludesStdOut = false;
ostream* MergeInfo_Out = nullptr;
ofstream Out;
static ostream* Log;
size_t Merge_Rewind_Count = 0;
uint8_t MergeInfo_Format = 0;
uint8_t Verbosity = 5;
uint64_t Timeout = 0;
uint8_t UseAbst = 0;
vector<bool> OutputFrames_Speeds;
vector<bool> OutputFrames_Concealeds;
bool OutputFrames_NoData = false;
int ShowFrames_Missing = -1;
int ShowFrames_Intermediate = -1;
extern void timecode_to_string(string& Data, int Seconds, bool DropFrame, int Frames);
void date_to_string(string& Data, int Years, int Months, int Days);
//---------------------------------------------------------------------------

namespace
{
    void Merge_Help()
    {
        if (MergeInfo_Format)
            return;
        if (Verbosity)
            *Log << NameVersion_Text() << '\n';
    }

    string TC_String(TimeCode const& TC)
    {
        if (!TC.HasValue())
            return "XX:XX:XX:XX";

        auto ToReturn = TC.ToString();
        if (ToReturn.empty() && !MergeInfo_Format)
            ToReturn.resize(11, ' ');

        return ToReturn;
    }

    string Abst_String(int const& Abst)
    {
        if (Abst == numeric_limits<int>::max())
            return "      ";

        auto ToReturn = to_string(Abst);
        if (ToReturn.size() < 6 && !MergeInfo_Format)
            ToReturn.insert(0, 6 - ToReturn.size(), ' ');

        return ToReturn;
    }

    enum status : uint8_t
    {
        Status_TimeCodeIssue,   // Time code is missing
        Status_BlockIssue,      // Some blocks are missing
        Status_FrameMissing,    // All blocks are missing
        Status_Max
    };

    enum block_status : uint8_t
    {
        BlockStatus_Unk,              // Unknown
        BlockStatus_OK,
        BlockStatus_NOK,
        BlockStatus_Max
    };

    struct buffer
    {
        uint8_t*        Data = nullptr;
        size_t          Size = 0;
    };

    struct per_frame
    {
        bitset<Status_Max>  Status;
        TimeCode            TC;
        timecode            TC_SMPTE;
        rec_date_time       RecDateTime;
        abst_bf             AbstBf;
        int                 Abst = numeric_limits<int>::max();
        uint8_t*            BlockStatus = nullptr;
        buffer              Buffer;
        size_t              BlockStatus_Count = 0;
        uint8_t             RepeatCount = 0;
        int                 Speed = INT_MIN;
        int                 FullConcealed = false;
        bool                NoData = false;

        per_frame() = default;
        per_frame(status const& Status_, ::TimeCode const& TC_, uint8_t* const& BlockStatus_, size_t BlockStatus_Count_) :
            Status((unsigned long)(1 << (unsigned)Status_)), TC(TC_), BlockStatus(BlockStatus_), BlockStatus_Count(BlockStatus_Count_) {}
        per_frame(unsigned long const Status_, ::TimeCode const& TC_, uint8_t* const& BlockStatus_, size_t BlockStatus_Count_) :
            Status(Status_), TC(TC_), BlockStatus(BlockStatus_), BlockStatus_Count(BlockStatus_Count_) {}
    };

    struct per_segment
    {
        vector<per_frame>   Frames;
    };

    struct dv_data
    {
        const buffer move()
        {
            if (Data.empty())
                return buffer();
            auto Temp = Data[0];
            Data.erase(Data.begin());
            return Temp;
        }

        void push_back(const uint8_t* Buffer, size_t Buffer_Size)
        {
            Data.resize(Data.size() + 1);
            Data.back().Data = new uint8_t[Buffer_Size];
            memcpy(Data.back().Data, Buffer, Buffer_Size);
            Data.back().Size = Buffer_Size;
        }

        void pop_front()
        {
            if (Data.empty())
                return;
            delete[] Data[0].Data;
            Data.erase(Data.begin());
        }

        const buffer& front()
        {
            return Data[0];
        }

        bool empty()
        {
            return Data.empty();
        }

        void clear()
        {
            while (!empty())
            {
                pop_front();
            }
        }

    private:
        vector<buffer>      Data;
    };
    
    struct per_file
    {
        FILE*               F;
        vector<FILE*>       Copies;
        FILE*               F_Takes = nullptr;
        size_t              F_Takes_Start = -1;
        uint64_t            F_Pos = 0;
        uint8_t             OutputBuffer[144000 * 4];
        size_t              Count_Blocks[BlockStatus_Max] = {};
        size_t              Count_Blocks_NOK_Frames_NOK = 0;
        size_t              Count_Blocks_Missing = 0;
        size_t              Count_Blocks_Missing_Frames_NOK = 0;
        size_t              Count_Blocks_Used = 0;
        size_t              Count_Blocks_Used_Sav = 0;
        size_t              Count_Frames_OK = 0;
        size_t              Count_Frames_NOK = 0;
        size_t              Count_Frames_Missing = 0;
        size_t              Count_Frames_Repeated = 0;
        size_t              Count_Frames_Ignored_Speed = 0;
        size_t              Count_Frames_Ignored_Concealed = 0;
        bool                DoNotUseFile = false;
        bool                FirstTimeCodeFound = false;
        vector<per_segment> Segments;
        dv_data*            DV_Data = nullptr;

        ~per_file()
        {
            if (F)
                fclose(F);
            for (FILE* C : Copies)
                fclose(C);
            if (F_Takes)
            {
                if (Verbosity == 10)
                    cerr << "Debug: Take file not closed on time." << endl;
                fclose(F_Takes);
            }
            for (auto& Segment : Segments)
                for (auto& Frame : Segment.Frames)
                {
                    delete[] Frame.BlockStatus;
                    delete[] Frame.Buffer.Data;
                }
        }

        size_t Count_Blocks_Bad()
        {
            return Count_Blocks[BlockStatus_NOK] + Count_Blocks_Missing;
        }

        size_t Count_Frames_Bad()
        {
            return Count_Frames_NOK + Count_Frames_Missing;
        }

        size_t Count_Blocks_Bad_Frames_NOK()
        {
            return Count_Blocks_NOK_Frames_NOK + Count_Blocks_Missing_Frames_NOK;
        }
    };

    // Display
    const int Formating_FrameCount_Width = 6;
    const int Formating_BlockCount_Width = 9;
    const int Formating_FrameBlockCount_Width = 4;
    const int Formating_Precision = 6;
    void ShowFrames(size_t Count, size_t Total, const char* Next = nullptr, bool AlwaysShow = false)
    {
        if (!Count && !AlwaysShow)
            return;
        auto Percent = (float)Count / Total * 100;
        *Log << fixed << setw(Formating_FrameCount_Width) << Count << " = " << fixed << setw(Formating_Precision + 2) << setprecision(Formating_Precision - 2) << Percent << "% of frames";
        if (Next)
            *Log << Next;
    }
    void ShowBlocks(size_t Count, size_t Total, const char* Next = nullptr)
    {
        auto Percent = (float)Count / Total * 100;
        *Log << fixed << setw(Formating_BlockCount_Width) << Count << " = " << fixed << setw(Formating_Precision + 2) << setprecision(Formating_Precision - 2) << Percent << "% of blocks";
        if (Next)
            *Log << Next;
    }

    //---------------------------------------------------------------------------
    class dv_merge_private
    {
    public:
        ~dv_merge_private()
        {
            for (const auto Input : Inputs)
                delete Input;
        }

        void AddFrameAnalysis(size_t InputPos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData, float Speed);
        void AddFrameData(size_t InputPos, const uint8_t* Buffer, size_t Buffer_Size);
        void Finish();

    private:
        bool Init();
        bool ManageRepeatedFrame(size_t InputPos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData);
        bool AppendFrameToList(size_t InputPos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData);
        bool ManagePartialFrame(size_t InputPos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData);
        bool TcSyncStart();
        bool SyncEnd(bool Force = false);
        bool Process(float Speed);
        bool Stats();

        mutex Mutex;
        vector<per_file*> Inputs;
        per_file Output;
        size_t Segment_Pos = 0;
        size_t Frame_Pos = 0;
        size_t Count_Last_Missing_Frames = 0;
        size_t Count_Last_OK_Frames = 0;
        bool FirstTimeCodeFound = false;
        stringstream CurrentLine; // If Verbosity < 9, cache current line data so status line is displayed without conflict

        // Stats
        struct count
        {
            size_t OK = 0;
            size_t Recovered = 0;
            size_t NOK = 0;
            size_t Missing = 0;
            size_t Good()
            {
                return OK + Recovered;
            }
            size_t Bad()
            {
                return NOK + Missing;
            }
            size_t Total()
            {
                return Good() + Bad();
            }
        };
        count Count_Blocks;
        count Count_Frames;
        size_t Count_Frames_NOK_Blocks_Total = 0;

        // Auto-rewind
        bool   CanRewind = false;
        size_t Input_Rewind_Pos = 0;
        size_t FirstFirstBadFrame = -1;
        size_t FirstBadFrame = -1;
        size_t LastBadFrame = -1;
        count Count_Blocks_Sav;
        count Count_Frames_Sav;

    public:
        TimeCode RewindToTimeCode;
    };
    dv_merge_private Merge_Private;
}

//---------------------------------------------------------------------------
void dv_merge::AddFrameAnalysis(size_t InputPos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData, float Speed)
{
    Merge_Private.AddFrameAnalysis(InputPos, FrameData, Speed);
    RewindToTimeCode = Merge_Private.RewindToTimeCode;
    Merge_Private.RewindToTimeCode = TimeCode();
}

//---------------------------------------------------------------------------
void dv_merge::AddFrameData(size_t InputPos, const uint8_t* Buffer, size_t Buffer_Size)
{
    Merge_Private.AddFrameData(InputPos, Buffer, Buffer_Size);
    RewindToTimeCode = Merge_Private.RewindToTimeCode;
    Merge_Private.RewindToTimeCode = TimeCode();
}

//---------------------------------------------------------------------------
void dv_merge::Finish()
{
    Merge_Private.Finish();
}

//---------------------------------------------------------------------------
bool dv_merge_private::Init()
{
    if (!Inputs.empty())
        return false;
    auto Input_Count = Merge_InputFileNames.size() + Merge_Rewind_Count;

    // Out config
    for (size_t Pos=0; Pos<Merge_Out.size(); Pos++)
    {
        if (!Pos)
            Output.F = Merge_Out[Pos];
        else
            Output.Copies.push_back(Merge_Out[Pos]);
    }
    Log = MergeInfo_Out ? MergeInfo_Out : (Merge_OutputFileNames_IncludesStdOut ? &cerr : &cout);

    Merge_Help();

    Inputs.reserve(Input_Count);
    if (Verbosity > 5 && !MergeInfo_Format)
        *Log << '\n';
    for (auto const& Inputs_FileName : Merge_InputFileNames)
    {
        if (Verbosity > 5 && !MergeInfo_Format)
            *Log << "File " << &Inputs_FileName - &Merge_InputFileNames.front() << ": " << Inputs_FileName << '\n';
        auto Input = new per_file;
        if (!Inputs_FileName.empty() && Inputs_FileName != "-" && Inputs_FileName.find("device://") != 0)
        {
            if (Verbosity == 10)
                cerr << "Debug: opening (in) \"" << Inputs_FileName << "\"..." << endl;
            Input->F = fopen(Inputs_FileName.c_str(), "rb");
            if (Verbosity == 10)
                cerr << "Debug: opening (in) \"" << Inputs_FileName << "\"... Done." << endl;
        }
        Input->Segments.resize(1);
        Inputs.push_back(Input);
    }
    if (!MergeInfo_Format)
        for (size_t i = 0; i < Merge_Rewind_Count; i++)
            *Log << "File " << Merge_InputFileNames.size() + i << ": auto-rewind\n";

    if (Verbosity > 5)
    {
        stringstream Log_Line;

        if (MergeInfo_Format == 0)
        {
            Log_Line << '\n' << std::setw(Formating_FrameCount_Width - 1) << ' ' << "#|Abst  |HH:MM:SS:FF|U|S";
            if (Input_Count > 1)
            {
                Log_Line << 't'; // "Status"
                if (Input_Count > 2)
                    Log_Line << setw(Input_Count - 2) << ' ';
            }
            Log_Line << "|Comments";
        }
        if (MergeInfo_Format == 1)
        {
            Log_Line << "FramePos,abst,abst_r,abst_nc,tc,tc_r,tc_nc,rdt,rdt_r,rdt_nc,rec_start,rec_end,Used,Status,Comments,BlockErrors,BlockErrors_Even,IssueFixed";
            if (Verbosity > 5)
                Log_Line << ",SourceSpeed,FrameSpeed,InputPos,OutputPos";
            if (ShowFrames_Intermediate)
                Log_Line << ",RewindStatus";
        }
        *Log << Log_Line.str() << endl;;
    }

    if (Inputs.size() < 2 && Merge_InputFileNames.front().find("device://") == 0 && Merge_Rewind_Count)
    {
        Inputs.reserve(1 + Merge_Rewind_Count);
        CanRewind = true;
        for (size_t i = 0; i < Merge_Rewind_Count; i++)
        {
            auto Input = new per_file;
            Input->DoNotUseFile = true;
            Inputs.push_back(Input);
        }
    }

    return false;
}

//---------------------------------------------------------------------------
bool dv_merge_private::ManageRepeatedFrame(size_t InputPos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData)
{
    if (!Merge_InputFileNames.front().rfind("device://", 0))
        return false; // No discard of repeated frames in case of capture

    auto& Input = Inputs[InputPos];
    auto& Frames = Input->Segments[Segment_Pos].Frames;

    // Ignore if frame repetition was found
    if (Input->DoNotUseFile || (timecode(FrameData).Repeat() && !Frames.empty()))
    {
        if (!Input->DoNotUseFile && !Frames.empty())
        {
            if (Frames.back().RepeatCount == 0xFF) // Check if there are too many repetition
                Input->DoNotUseFile = true;
            else
            {
                Frames.back().RepeatCount++;
                Input->Count_Frames_Repeated++;
                if (Frame_Pos >= Frames.size()) // Frame already parsed
                {
                    if (Input->F)
                    {
                        if (fseek(Input->F, (long)(Input->Segments[Segment_Pos].Frames.back().BlockStatus_Count * 80), SEEK_CUR))
                            *Log << "File seek issue." << endl;
                    }
                    else if (Input->DV_Data && !Input->DV_Data->empty())
                    {
                        Input->DV_Data->pop_front();
                    }
                }
            }
        }
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------
bool dv_merge_private::AppendFrameToList(size_t InputPos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData)
{
    auto& Input = Inputs[InputPos];
    auto& Frames = Input->Segments.back().Frames;
    auto BlockStatus_Count = FrameData->BlockStatus_Count;
    per_frame CurrentFrame;

    // Absolute track number
    abst_bf AbstBf_Temp(FrameData->AbstBf);
    if (AbstBf_Temp.HasAbsoluteTrackNumberValue())
    {
        CurrentFrame.Abst = AbstBf_Temp.AbsoluteTrackNumber();
    }

    // Fill informational data
    CurrentFrame.RecDateTime = rec_date_time(FrameData);
    CurrentFrame.TC_SMPTE = timecode(FrameData);
    CurrentFrame.AbstBf = abst_bf(FrameData->AbstBf);
    CurrentFrame.Speed = GetDvSpeed(*FrameData);
    CurrentFrame.FullConcealed = coherency_flags(FrameData).full_conceal();
    CurrentFrame.NoData = coherency_flags(FrameData).no_data();

    // Time code jumps - after first frame
    timecode TC_Temp(FrameData);
    if (TC_Temp.HasValue())
        CurrentFrame.TC = TimeCode(TC_Temp.TimeInSeconds() / 3600, (TC_Temp.TimeInSeconds() / 60) % 60, TC_Temp.TimeInSeconds() % 60, TC_Temp.Frames(), 30 /*TEMP*/, TC_Temp.DropFrame());
    if (!Frames.empty() && Frames.back().TC.HasValue())
    {
        TimeCode TC_Previous(Frames.back().TC);
        if (CurrentFrame.TC.HasValue())
        {
            if (CurrentFrame.TC.ToFrames() < TC_Previous.ToFrames())
            {
                Input->Segments.resize(Input->Segments.size() + 1);
                Input->Segments.back().Frames.emplace_back(move(CurrentFrame));
                return false;
            }
            if (CurrentFrame.TC.ToFrames() != TC_Previous.ToFrames()) // Accept repeated frames
                TC_Previous++;
            while (TC_Previous != CurrentFrame.TC)
            {
                Frames.emplace_back(Status_FrameMissing, TC_Previous, nullptr, BlockStatus_Count);
                Input->Count_Blocks_Missing += BlockStatus_Count;
                Input->Count_Frames_Missing++;
                TC_Previous++;
            }
        }
        else
        {
            CurrentFrame.TC = TC_Previous + 1;
            CurrentFrame.Status.set(Status_TimeCodeIssue);
        }
    }

    Frames.emplace_back(move(CurrentFrame));

    return false;
}

//---------------------------------------------------------------------------
bool dv_merge_private::ManagePartialFrame(size_t InputPos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData)
{
    auto& Input = Inputs[InputPos];
    auto& Frames = Input->Segments.back().Frames;
    auto& CurrentFrame = Frames.back();
    auto BlockStatus_Count = FrameData->BlockStatus_Count;

    // Mergeable blocks
    CurrentFrame.BlockStatus_Count = BlockStatus_Count;
    size_t Count_Blocks[BlockStatus_Max] = {};
    for (int i = 0; i < BlockStatus_Count; i++)
    {
        auto BlockStatus = FrameData->BlockStatus[i];
        if (BlockStatus < BlockStatus_Max)
            Count_Blocks[BlockStatus]++;
    }
    if (Count_Blocks[BlockStatus_NOK])
    {
        CurrentFrame.Status.set(Status_BlockIssue);
        CurrentFrame.BlockStatus = new uint8_t[BlockStatus_Count];
        memcpy(CurrentFrame.BlockStatus, FrameData->BlockStatus, BlockStatus_Count);
        Input->Count_Frames_NOK++;
    }
    else
    {
        Input->Count_Frames_OK++;
    }
    for (int i = 0; i < BlockStatus_Max; i++)
    {
        Input->Count_Blocks[i] += Count_Blocks[i];
    }

    return false;
}

//---------------------------------------------------------------------------
bool dv_merge_private::TcSyncStart()
{
    auto Input_Count = Inputs.size();

    // Time code jumps - first frame
    if (Frame_Pos)
        return false;

    // Look for first time code presence per input and minimum theoritical time code
    vector<size_t> StartPos;
    int64_t TC_Min = numeric_limits<int64_t>::max();
    for (size_t i = 0; i < Input_Count; i++)
    {
        auto& Input = Inputs[i];
        if (Segment_Pos >= Input->Segments.size())
            continue;
        auto& Frames = Input->Segments[Segment_Pos].Frames;

        size_t Frames_Pos = 0;
        while (Frames_Pos < Frames.size() && !Frames[Frames_Pos].TC.HasValue())
            Frames_Pos++;
        if (Frames_Pos >= Frames.size())
            return true;
        StartPos.push_back(Frames_Pos);
        int64_t TC_Min_ThisInput = Frames[Frames_Pos].TC.ToFrames() - Frames_Pos;
        if (TC_Min > TC_Min_ThisInput)
            TC_Min = TC_Min_ThisInput;
    }

    // Align inputs
    for (size_t i = 0; i < Input_Count; i++)
    {
        auto& Input = Inputs[i];
        if (Segment_Pos >= Input->Segments.size())
            continue;
        auto& Frames = Input->Segments[Segment_Pos].Frames;

        auto MissingFrames = Frames[StartPos[i]].TC.ToFrames() - StartPos[i] - TC_Min;
        if (!MissingFrames)
            continue;
        Input->Count_Blocks_Missing += Frames[StartPos[i]].BlockStatus_Count * MissingFrames;
        Input->Count_Frames_Missing += MissingFrames;
        per_frame PreviousFrame;
        PreviousFrame.Status.set(Status_FrameMissing);
        PreviousFrame.BlockStatus_Count = Frames[StartPos[i]].BlockStatus_Count;
        Frames.insert(Frames.begin(), MissingFrames, PreviousFrame);
    }
    return false;
}

//---------------------------------------------------------------------------
bool dv_merge_private::SyncEnd(bool Force)
{
    auto Input_Count = Inputs.size();
    size_t Segments_Max = Segment_Pos + 1;

    if (Force)
    {
        // Segments
        for (size_t i = 0; i < Input_Count; i++)
        {
            auto& Input = Inputs[i];
            if (Input->DoNotUseFile)
                continue;
            if (Segments_Max < Input->Segments.size())
            {
                Segments_Max = Input->Segments.size();
                per_frame MissingFrame;
                MissingFrame.Status.set(Status_FrameMissing);
                MissingFrame.TC = Input->Segments.back().Frames[0].TC;
                auto BlockStatus_Count = Input->Segments.back().Frames[0].BlockStatus_Count;
                for (size_t j = 0; j < Input_Count; j++)
                {
                    auto& Input = Inputs[j];
                    if (Input->DoNotUseFile)
                        continue;
                    if (Input->Segments.size() < Segments_Max)
                    {
                        Input->Segments.resize(Segments_Max);
                        Input->Segments.back().Frames.push_back(MissingFrame);
                        Input->Count_Blocks_Missing += BlockStatus_Count;
                        Input->Count_Frames_Missing++;
                    }
                }
            }
        }
    }

    // Check which frames are available at least once
    for (auto Segment_PosTemp = Segment_Pos; Segment_PosTemp < Segments_Max; Segment_PosTemp++)
    {
        size_t LongestFile = 0;
        size_t Frames_Status_Max = 0;
        size_t BlockStatus_Count = 0;
        for (size_t i = 0; i < Input_Count; i++)
        {
            auto& Input = Inputs[i];
            if (Input->DoNotUseFile)
                continue;
            auto& Frames = Input->Segments[Segment_PosTemp].Frames;
            if (Frames_Status_Max < Frames.size())
            {
                Frames_Status_Max = Frames.size();
                BlockStatus_Count = Frames.back().BlockStatus_Count;
            }
        }
        for (size_t i = 0; i < Input_Count; i++)
        {
            auto& Input = Inputs[i];
            if (Input->DoNotUseFile)
                continue;
            auto& Frames = Input->Segments[Segment_PosTemp].Frames;
            while (Frames.size() < Frames_Status_Max)
            {
                Frames.emplace_back(Status_FrameMissing, TimeCode(), nullptr, BlockStatus_Count);
                Input->Count_Blocks_Missing += BlockStatus_Count;
                Input->Count_Frames_Missing++;
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------
bool dv_merge_private::Process(float Speed)
{
    auto Input_Count = Inputs.size();
    size_t Frames_Status_Max;
    for (int i = 0; i < 2; i++)
    {
        Frames_Status_Max = -1;
        for (size_t i = 0; i < Input_Count; i++)
        {
            auto& Input = Inputs[i];
            if (Input->DoNotUseFile)
                continue;
            auto& Frames = Input->Segments[Segment_Pos].Frames;
            if (Frames_Status_Max > Frames.size())
                Frames_Status_Max = Frames.size();
        }
        if (Frame_Pos < Frames_Status_Max)
            break;
        auto Segment_Pos1 = Segment_Pos + 1;
        for (const auto Input : Inputs)
        {
            if (Segment_Pos1 >= Input->Segments.size())
                return true;
        }
        if (SyncEnd())
            return true;
        if (!i)
            continue; // Second pass of the current segment
        i = 0;
        Segment_Pos++;
        Frame_Pos = 0;
        if (TcSyncStart())
            return true;
    }
    size_t MaxSegmentSize = 0;
    for (const auto Input : Inputs)
    {
        if (MaxSegmentSize < Input->Segments.size())
            MaxSegmentSize = Input->Segments.size();
    }
    if (Segment_Pos >= MaxSegmentSize || Frame_Pos >= Frames_Status_Max)
        return true;

    // Check abst coherency
    if (UseAbst && Inputs.size() == 2) // Currently limited to 2 inputs
    {
        set<int> abst_List;
        for (size_t i = 0; i < Input_Count; i++)
        {
            auto& Input = Inputs[i];
            auto& Frames = Input->Segments[Segment_Pos].Frames;
            auto& Frame = Frames[Frame_Pos];
            if (Frame.Abst != numeric_limits<int>::max())
                abst_List.insert(Frame.Abst);
        }
        if (abst_List.size() > 1)
        {
            // Find the reference input: if no previous frame it is the one with the smallest abst, else the one with missing frames before the current one
            size_t RefInput = -1;
            if (Frame_Pos)
            {
                for (size_t i = 0; i < Input_Count; i++)
                {
                    const auto& Input = Inputs[i];
                    if (!Input->Segments[Segment_Pos].Frames[Frame_Pos - 1].Status[Status_FrameMissing])
                    {
                        RefInput = i;
                        break;
                    }
                }
            }
            else
            {
                int MinAbst = numeric_limits<int>::max();
                for (size_t i = 0; i < Input_Count; i++)
                {
                    const auto& Input = Inputs[i];
                    if (MinAbst > Input->Segments[Segment_Pos].Frames[0].Abst)
                    {
                        MinAbst = Input->Segments[Segment_Pos].Frames[0].Abst;
                        RefInput = i;
                    }
                }
            }

            if (RefInput != -1)
            {
                for (size_t i = 0; i < Input_Count; i++)
                {
                    if (i == RefInput)
                        continue;
                    auto& Input = Inputs[i];
                    auto& Frames = Input->Segments[Segment_Pos].Frames;
                    auto& Frame = Frames[Frame_Pos];
                    Input->Segments.insert(Input->Segments.begin() + Segment_Pos, per_segment());
                    auto& Frames1 = Input->Segments[Segment_Pos].Frames;
                    auto& Frames2 = Input->Segments[Segment_Pos + 1].Frames;
                    Frames1.insert(Frames1.begin(), Frames2.begin(), Frames2.begin() + Frame_Pos);
                    Frames2.erase(Frames2.begin(), Frames2.begin() + Frame_Pos);
                }
            }

            return true;
        }
    }

    // Check if there is a single frame available for this time code
    size_t IsMissing = Input_Count;
    size_t IsOK = Input_Count;
    for (size_t i = 0; i < Input_Count; i++)
    {
        auto& Input = Inputs[i];
        if (Input->DoNotUseFile)
            continue;
        auto& Frames = Input->Segments[Segment_Pos].Frames;
        auto& Frame = Frames[Frame_Pos];
        if (!Frame.Status[Status_FrameMissing])
            IsMissing--;
        if (Frame.Status.any())
            IsOK--;
    }
    if (IsMissing < Input_Count)
        IsMissing = 0;
    if (Count_Last_Missing_Frames && !IsMissing && Verbosity <= 7)
    {
        if (Verbosity > 5 && Count_Last_Missing_Frames)
        {
            if (Count_Last_Missing_Frames > 1)
            {
                CurrentLine << setw(Input_Count + 2) << ' ' << '(' << setw(Formating_FrameCount_Width) << Count_Last_Missing_Frames << " frames)";
            }
            *Log << CurrentLine.str() << endl;
            CurrentLine.str(string());
        }
        Count_Last_Missing_Frames = 0;
    }
    if (IsOK < Input_Count)
        IsOK = 0;
    if (Count_Last_OK_Frames && !IsOK && Verbosity <= 7)
    {
        if (Verbosity > 5 && Count_Last_OK_Frames)
        {
            if (Count_Last_OK_Frames > 1)
            {
                CurrentLine << setw(Input_Count + 2) << ' ' << '(' << setw(Formating_FrameCount_Width) << Count_Last_OK_Frames << " frames)";
            }
            *Log << CurrentLine.str() << endl;
            CurrentLine.str(string());
        }
        Count_Last_OK_Frames = 0;
    }

    stringstream Log_Line;
    if (Verbosity > 5 && !Count_Last_Missing_Frames && !Count_Last_OK_Frames)
    {
        auto& Out = (Verbosity <= 7 && !(!IsMissing && !IsOK)) ? CurrentLine : Log_Line;
        if (!MergeInfo_Format)
            Out << setfill(' ') << setw(Formating_FrameCount_Width);
        Out << Count_Frames.Total();
    }

    size_t BlockStatus_Count = 0;
    for (size_t i = 0; i < Input_Count; i++)
    {
        auto& Input = Inputs[i];
        if (Input->DoNotUseFile)
            continue;
        auto& Frames = Input->Segments[Segment_Pos].Frames;
        auto& Frame = Frames[Frame_Pos];
        if (BlockStatus_Count < Frame.BlockStatus_Count)
            BlockStatus_Count = Frame.BlockStatus_Count;
    }

    // Copy 1 frame in memory, for each input
    vector<uint64_t> Input_Previous_F_Pos;
    for (size_t i = 0; i < Input_Count; i++)
    {
        auto& Input = Inputs[i];
        if (Input->DoNotUseFile)
            continue;
        auto& Frames = Input->Segments[Segment_Pos].Frames;
        auto& Frame = Frames[Frame_Pos];
        if (!Frame.Status[Status_FrameMissing])
        {
            Input_Previous_F_Pos.push_back(Input->F_Pos);
            if (Input->F)
            {
                auto Frame_Size = Frame.BlockStatus_Count * 80;
                Frame.Buffer.Data = new int8u[Frame_Size];
                Frame.Buffer.Size = Frame_Size;
                auto BytesRead = fread(Frame.Buffer.Data, 1, Frame_Size, Input->F);
                Frame.Buffer.Size = BytesRead;
                Input->F_Pos += BytesRead;
                if (Frame.Buffer.Size != Frame.BlockStatus_Count * 80)
                {
                    *Log << "File read issue." << endl;
                    Frame.BlockStatus_Count = Frame.Buffer.Size / 80;
                }
                if (Frame.RepeatCount)
                {
                    if (fseek(Input->F, (long)(Frame.BlockStatus_Count * 80 * Frame.RepeatCount), SEEK_CUR))
                        *Log << "File seek issue." << endl;
                }
            }
            else if (Input->DV_Data && !Input->DV_Data->empty())
            {
                //memcpy(Frame.Buffer, Input->DV_Data->front().Data, Input->DV_Data->front().Size); 
                Frame.Buffer = Input->DV_Data->move();
                Input->F_Pos += Frame.Buffer.Size;
                if (Frame.Buffer.Size != Frame.BlockStatus_Count * 80)
                {
                    *Log << "File read issue." << endl;
                    Frame.BlockStatus_Count = Frame.Buffer.Size / 80;
                }
            }
        }
        else
        {
            Input_Previous_F_Pos.push_back((uint64_t) - 1);
        }
    }

    // Find a prefered frame
    size_t Prefered_Frame = -1;
    size_t Prefered_TC = -1;
    size_t Prefered_Abst = -1;
    for (size_t i = 0; i < Input_Count; i++)
    {
        auto& Input = Inputs[i];
        if (Input->DoNotUseFile)
            continue;
        auto& Frames = Input->Segments[Segment_Pos].Frames;
        auto& Frame = Frames[Frame_Pos];
        if (Prefered_Frame == -1 && !Frame.Status[Status_FrameMissing] && !Frame.Status[Status_BlockIssue])
            Prefered_Frame = i;
        if (Prefered_TC == -1 && !Frame.Status[Status_TimeCodeIssue] && Frame.TC.HasValue())
            Prefered_TC = i;
        if (Prefered_Abst != -2 && Frame.Abst != numeric_limits<int>::max())
        {
            if (Prefered_Abst == -1)
                Prefered_Abst = i;
            else
            {
                auto& Input2 = Inputs[Prefered_Abst];
                auto& Frames2 = Input2->Segments[Segment_Pos].Frames;
                auto& Frame2 = Frames2[Frame_Pos];
                if (Frame2.Abst != Frame.Abst)
                    Prefered_Abst = -2; // Incoherency
            }
        }
    }

    char Log_Separator = MergeInfo_Format ? ',' : ' ';
    if (Verbosity > 5 && !Count_Last_Missing_Frames && !Count_Last_OK_Frames)
    {
        auto& Out = (Verbosity <= 7 && !(!IsMissing && !IsOK)) ? CurrentLine : Log_Line;
        if (!MergeInfo_Format)
        {
            if (Prefered_Abst == -2)
            {
                Out << Log_Separator << "??????";
            }
            else if (Prefered_Abst != -1)
            {
                auto& Input = Inputs[Prefered_Abst];
                auto& Frames = Input->Segments[Segment_Pos].Frames;
                auto& Frame = Frames[Frame_Pos];
                Out << Log_Separator << Abst_String(Frame.Abst);
            }
            else if (MergeInfo_Format)
                Out << ',';
            else
                Out << "       ";
            if (Prefered_TC != -1)
            {
                auto& Input = Inputs[Prefered_TC];
                auto& Frames = Input->Segments[Segment_Pos].Frames;
                auto& Frame = Frames[Frame_Pos];
                Out << Log_Separator << TC_String(Frame.TC);
            }
            else
                Out << Log_Separator << "??:??:??:??";
        }
        else
        {
            size_t Prefered_Info;
            if (Prefered_Frame == -1)
            {
                Prefered_Info = Input_Count;
                for (size_t i = 0; i < Input_Count; i++)
                {
                    auto& Input = Inputs[i];
                    auto& Frames = Input->Segments[Segment_Pos].Frames;
                    auto& Frame = Frames[Frame_Pos];
                    if (!Frame.Status[Status_FrameMissing])
                    {
                        Prefered_Info = i;
                        break;
                    }
                }
                if (Prefered_Info == Input_Count)
                    Prefered_Info = 0; // by default
            }
            else
                Prefered_Info = Prefered_Frame;
            auto& Input = Inputs[Prefered_Info];
            auto& Frames = Input->Segments[Segment_Pos].Frames;
            auto& Frame = Frames[Frame_Pos];
            string Temp;
            Out << Log_Separator;
            if (!Frame.Status[Status_FrameMissing] && Frame.AbstBf.HasAbsoluteTrackNumberValue())
            {
                Out << to_string(Frame.AbstBf.AbsoluteTrackNumber());
            }
            Out << Log_Separator;
            if (!Frame.Status[Status_FrameMissing] && Frame.AbstBf.Repeat())
            {
                Out << '1';
            }
            Out << Log_Separator;
            if (!Frame.Status[Status_FrameMissing] && Frame.AbstBf.NonConsecutive())
            {
                Out << '1';
            }
            if (!Frame.Status[Status_FrameMissing] && Frame.TC_SMPTE.HasValue())
                timecode_to_string(Temp, Frame.TC_SMPTE.TimeInSeconds(), Frame.TC_SMPTE.DropFrame(), Frame.TC_SMPTE.Frames());
            Out << Log_Separator << Temp;
            Out << Log_Separator;
            if (!Frame.Status[Status_FrameMissing] && Frame.TC_SMPTE.Repeat())
            {
                Out << '1';
            }
            Out << Log_Separator;
            if (!Frame.Status[Status_FrameMissing] && Frame.TC_SMPTE.NonConsecutive())
            {
                if (Frame.TC_SMPTE.NonConsecutive_IsLess())
                    Out << '2';
                else
                    Out << '1';
            }
            Temp.clear();
            if (!Frame.Status[Status_FrameMissing] && Frame.RecDateTime.HasDate())
            {
                date_to_string(Temp, Frame.RecDateTime.Years(), Frame.RecDateTime.Months(), Frame.RecDateTime.Days());
            }
            if (!Frame.Status[Status_FrameMissing] && Frame.RecDateTime.HasTime())
            {
                if (!Temp.empty())
                    Temp += ' ';
                timecode_to_string(Temp, Frame.RecDateTime.TimeInSeconds(), Frame.TC_SMPTE.DropFrame(), Frame.RecDateTime.Frames());
            }
            Out << Log_Separator << Temp;
            Out << Log_Separator;
            if (!Frame.Status[Status_FrameMissing] && Frame.RecDateTime.Repeat())
            {
                Out << '1';
            }
            Out << Log_Separator;
            if (!Frame.Status[Status_FrameMissing] && Frame.RecDateTime.NonConsecutive())
            {
                if (Frame.RecDateTime.NonConsecutive_IsLess())
                    Out << '2';
                else
                    Out << '1';
            }
            Out << Log_Separator;
            if (!Frame.Status[Status_FrameMissing] && Frame.RecDateTime.Start())
            {
                Out << '1';
            }
            Out << Log_Separator;
            if (!Frame.Status[Status_FrameMissing] && Frame.RecDateTime.End())
            {
                Out << '1';
            }
        }
        if (Prefered_Frame != -1)
            Out << Log_Separator << Prefered_Frame;
        else if (IsMissing)
            Out << Log_Separator << 'M';
        else
            Out << Log_Separator << 'X';
    }
    
    if (IsMissing)
    {
        auto& Input = Inputs[0];
        auto& Frames = Input->Segments[Segment_Pos].Frames;
        auto& Frame = Frames[Frame_Pos];
        Count_Blocks.Missing += Frame.BlockStatus_Count;
        Count_Frames.Missing++; // Don't try to find good blocks if there is no file with good blocks
        Frame_Pos++;
        if (Verbosity <= 7)
            Count_Last_Missing_Frames++;
        else if (true
            && (ShowFrames_Missing || !IsMissing)
            )
        {
            *Log << Log_Line.str() << endl;
        }
        return false;
    }

    if (IsOK)
    {
        if (Verbosity <= 7)
            Count_Last_OK_Frames++;
    }

    // Find valid blocks
    // Status
    size_t Text_Status_ToFill = 0;
    if (MergeInfo_Format)
        Log_Line << Log_Separator;
    if (!IsMissing && !IsOK)
    {
        if (!MergeInfo_Format)
        {
            Text_Status_ToFill = 1;
            Log_Line << Log_Separator;
        }
        for (size_t i = 0; i < Input_Count; i++)
        {
            auto& Input = Inputs[i];
            if (Input->DoNotUseFile)
            {
                Log_Line << ' ';
                continue;
            }
            auto& Frames = Input->Segments[Segment_Pos].Frames;
            auto& Frame = Frames[Frame_Pos];
            if (Frame.Status[Status_FrameMissing])
                Log_Line << 'M';
            else if (Frame.Status[Status_BlockIssue])
                Log_Line << 'P';
            else if (Frame.Status[Status_TimeCodeIssue])
                Log_Line << 'T';
            else
                Log_Line << ' ';
        }
    }
    else
        Text_Status_ToFill = 1 + Input_Count + 1;

    // Find valid blocks
    size_t IssueCount = 0;
    size_t IssueCount_Even = 0;
    size_t IssueFixed = 0;
    if (Verbosity > 5 && MergeInfo_Format)
        Log_Line << ',';
    if (Prefered_Frame != -1)
    {
        auto& Input = Inputs[Prefered_Frame];
        auto& Frames = Input->Segments[Segment_Pos].Frames;
        auto& Frame = Frames[Frame_Pos];
        Input->Count_Blocks_Used += BlockStatus_Count;
        Count_Blocks.OK += BlockStatus_Count;
        Count_Frames.OK++;
        memcpy(Output.OutputBuffer, Frame.Buffer.Data, Frame.BlockStatus_Count * 80);
    }
    else
    {
        size_t ThisFrame_Count_Blocks_Used[100];
        memset(ThisFrame_Count_Blocks_Used, 0, Input_Count * sizeof(size_t));

        // For each block
        vector<size_t> Priorities; // Find the frame with the minimum count of bad blocks
        Priorities.resize(Input_Count);
        for (int b = 0; b < BlockStatus_Count; b++)
        {
            for (int i = 0; i < Input_Count; i++)
            {
                auto& Input = Inputs[i];
                if (Input->DoNotUseFile)
                {
                    Priorities[i] = (size_t)-1;
                    continue;
                }
                auto& Frames = Input->Segments[Segment_Pos].Frames;
                auto& Frame = Frames[Frame_Pos];
                if (Frame.Status[Status_BlockIssue])
                {
                    switch (Frame.BlockStatus[b])
                    {
                    case BlockStatus_NOK:
                        Priorities[i]++;
                        Input->Count_Blocks_NOK_Frames_NOK++;
                        break;
                    default:;
                    }
                }
                else if (!b)
                    Priorities[i] = (size_t)-1;
            }
        }
        map<size_t, vector<size_t>> Priorities2;
        for (int i = 0; i < Input_Count; i++)
            Priorities2[Priorities[i]].push_back(i);
        Priorities.clear();
        for (auto p : Priorities2)
            for (auto k : p.second)
                Priorities.push_back(k);
        {
            for (size_t i = 0; i < Input_Count; i++)
            {
                auto& Input = Inputs[i];
                if (Input->DoNotUseFile)
                    continue;
                auto& Frames = Input->Segments[Segment_Pos].Frames;
                auto& Frame = Frames[Frame_Pos];
                if (Frame.Status[Status_FrameMissing])
                    Input->Count_Blocks_Missing_Frames_NOK += BlockStatus_Count;
            }
            Prefered_Frame = Priorities[0];
            Inputs[Prefered_Frame]->Count_Blocks_Used += BlockStatus_Count;
            memcpy(Output.OutputBuffer, Inputs[Priorities[0]]->Segments[Segment_Pos].Frames[Frame_Pos].Buffer.Data, BlockStatus_Count * 80); // Copy the content of the file having the less issues
            for (int b = 0; b < BlockStatus_Count; b++)
            {
                bool NoIssue = false;
                for (int i = 0; i < Input_Count; i++)
                {
                    auto p = Priorities[i];
                    auto& Input = Inputs[p];
                    if (Input->DoNotUseFile)
                        continue;
                    auto& Frames = Input->Segments[Segment_Pos].Frames;
                    auto& Frame = Frames[Frame_Pos];
                    if (Frame.Status[Status_BlockIssue])
                    {
                        switch (Frame.BlockStatus[b])
                        {
                        case 1:
                            if (!NoIssue)
                            {
                                if (i)
                                    memcpy(Output.OutputBuffer + b * 80, Frame.Buffer.Data + b * 80, 80);
                                ThisFrame_Count_Blocks_Used[p]++;
                            }
                            NoIssue = true;
                            break;
                        case 2:
                            break;
                        default:;
                            NoIssue = true;
                        }
                    }
                }
                if (!NoIssue)
                {
                    IssueCount++;
                    auto Dseq = Output.OutputBuffer[b * 80 + 1] >> 4;
                    if (!(Dseq % 2))
                        IssueCount_Even++;
                }
            }
            if (Verbosity > 5 && !MergeInfo_Format)
            {
                if (Text_Status_ToFill)
                {
                    Log_Line << setw(Text_Status_ToFill) << Log_Separator;
                    Text_Status_ToFill = 0;
                }
                else
                {
                    Log_Line << ", ";
                }
            }
            for (int i = 0; i < Input_Count; i++)
            {
                if (i != Prefered_Frame)
                {
                    auto& Input = Inputs[i];
                    Input->Count_Blocks_Used += ThisFrame_Count_Blocks_Used[i];
                    Inputs[Prefered_Frame]->Count_Blocks_Used -= ThisFrame_Count_Blocks_Used[i];
                    IssueFixed += ThisFrame_Count_Blocks_Used[i];
                }
                if (Verbosity > 5)
                {
                    if (i)
                        Log_Line << ' ';
                    if (CanRewind && !ThisFrame_Count_Blocks_Used[i] && i > Input_Rewind_Pos)
                        Log_Line << fixed << setw(Formating_FrameBlockCount_Width) << ' ';
                    else
                        Log_Line << fixed << setw(Formating_FrameBlockCount_Width) << ThisFrame_Count_Blocks_Used[i];
                }
            }
            if (Verbosity > 5)
                Log_Line << " block picks";
            if (IssueCount)
            {
                if (Verbosity > 5)
                    Log_Line << " & " << fixed << setw(Formating_FrameBlockCount_Width) << IssueCount << " remaining block errors";

                // Check if we should plan to rewind
                if (CanRewind && Input_Rewind_Pos + 1 < Inputs.size() && FirstBadFrame == -1)
                {
                    FirstBadFrame = Frame_Pos;
                    if (FirstFirstBadFrame == -1)
                        FirstFirstBadFrame = FirstBadFrame;

                    // Save stats
                    Count_Blocks_Sav = Count_Blocks;
                    Count_Frames_Sav = Count_Frames;
                    for (const auto Input : Inputs)
                        Input->Count_Blocks_Used_Sav = Input->Count_Blocks_Used;
                }

                Count_Frames.NOK++;
                Count_Frames_NOK_Blocks_Total += BlockStatus_Count;
            }
            else
            {
                Count_Frames.Recovered++;
            }
            Count_Blocks.OK += BlockStatus_Count - IssueCount;
            Count_Blocks.NOK += IssueCount;
        }
    }

    if (MergeInfo_Format)
    {
        Log_Line << ',';
        Log_Line << IssueCount; // Remaining block errors for this frame
        Log_Line << ',';
        Log_Line << IssueCount_Even; // Remaining block errors for this frame (even Dseq)
        Log_Line << ',';
        Log_Line << IssueFixed; // Corrections of blocks for this frame
    }

    if (Verbosity > 5 && Prefered_Abst == -2 && !MergeInfo_Format)
    {
        if (Text_Status_ToFill)
        {
            Log_Line << setw(Text_Status_ToFill) << Log_Separator;
            Text_Status_ToFill = 0;
        }
        else
        {
            Log_Line << ", ";
        }
        Log_Line << "abst";
        for (size_t i = 0; i < Input_Count; i++)
        {
            auto& Input = Inputs[i];
            if (Input->DoNotUseFile)
                continue;
            auto& Frames = Input->Segments[Segment_Pos].Frames;
            auto& Frame = Frames[Frame_Pos];
            if (Frame.Abst == numeric_limits<int>::max())
            {
                Log_Line << (Frame.Status[Status_FrameMissing] ? "       " : " missin"); // missing abst but only 6 chars for abst
            }
            else
            {
                Log_Line << Log_Separator << Abst_String(Frame.Abst);
            }
        }
    }
    
    if (Verbosity > 5 && !IsOK)
    {
        bool HasTimeCodeIssue = false;
        for (size_t i = 1; i < Input_Count; i++)
        {
            auto& Input = Inputs[i];
            if (Input->DoNotUseFile)
                continue;
            auto& Frames = Input->Segments[Segment_Pos].Frames;
            auto& Frame = Frames[Frame_Pos];
            if (Frame.Status[Status_TimeCodeIssue])
                HasTimeCodeIssue = true;
        }
        if (HasTimeCodeIssue)
        {
            if (Verbosity > 5 && !MergeInfo_Format)
            {
                Log_Line << ',';
                for (size_t i = 0; i < Input_Count; i++)
                {
                    Log_Line << Log_Separator;
                    auto& Input = Inputs[i];
                    auto& Frames = Input->Segments[Segment_Pos].Frames;
                    auto& Frame = Frames[Frame_Pos];
                    Log_Line << (Frame.Status[Status_TimeCodeIssue] ? "??:??:??:??" : TC_String(Frame.TC));
                }
            }
        }
    }

    auto& Input = Inputs[0];
    auto& Frames = Input->Segments[Segment_Pos].Frames;
    auto& Frame = Frames[Frame_Pos];
    auto DvSpeed = Frame.Speed;
    if (MergeInfo_Format)
    {
        Log_Line << ',';
        Log_Line << ::to_string(Speed);
        Log_Line << ',';
        if (DvSpeed != INT_MIN)
            Log_Line << std::to_string(DvSpeed);
        Log_Line << ',';
        if (Prefered_Frame != -1)
            for (size_t i = 0; i < Input_Previous_F_Pos.size(); i++)
            {
                if (i)
                    Log_Line << '|';
                if (Input_Previous_F_Pos[i] != (uint64_t)-1)
                    Log_Line << Input_Previous_F_Pos[i];
            }
        Log_Line << ',';
        if (Prefered_Frame != -1 && FirstBadFrame == -1)
            Log_Line << Output.F_Pos;
        if (ShowFrames_Intermediate)
        {
            Log_Line << ',';
            if (FirstBadFrame != -1)
                Log_Line << 'R';
        }
    }

    // Auto-rewind
    if (CanRewind && FirstBadFrame != -1)
    {
        bool ShouldRewind = Input_Rewind_Pos + 1 < Inputs.size() && (Frame_Pos > LastBadFrame || (LastBadFrame == (size_t)-1 && !IssueCount));
        if (ShouldRewind)
        {
            // Reset stats
            Count_Blocks = Count_Blocks_Sav;
            Count_Frames = Count_Frames_Sav;
            for (const auto Input : Inputs)
                Input->Count_Blocks_Used = Input->Count_Blocks_Used_Sav;

            // Prepare rewind
            if (LastBadFrame == (size_t)-1)
                LastBadFrame = Frame_Pos - 1;
            Frame_Pos = FirstBadFrame;
            FirstBadFrame = -1;
            Input_Rewind_Pos++;
            auto& Input = Inputs[Input_Rewind_Pos];
            if (Input->DV_Data)
                Input->DV_Data->clear();
            Input->DoNotUseFile = false;
            Input->Segments.resize(Inputs[Input_Rewind_Pos - 1]->Segments.size());
            Input->Segments[Input->Segments.size() - 1].Frames.resize(Frame_Pos);
            *Log << Log_Line.str() << endl;
            if (Verbosity >= 5 && !MergeInfo_Format && ShowFrames_Intermediate)
            {
                *Log << "Rewind to frame " << Frame_Pos << endl;
            }

            // Rewind
            auto& Input0 = Inputs[0];
            auto& Frames0 = Input0->Segments[Segment_Pos].Frames;
            auto& Frame0 = Frames0[Frame_Pos];
            RewindToTimeCode = Frame0.TC;

            for (size_t i = 0; i < Inputs.size() - 1; i++)
            {
                auto& Input = Inputs[i];
                if (Input->F_Takes)
                {
                    fclose(Input->F_Takes);
                    remove((Merge_OutputFileNames[0] + ".devrescue.take" + to_string(i) + ".frames" + to_string(Input->F_Takes_Start) + '-' + to_string(LastBadFrame) + ".dv").c_str());
                    rename((Merge_OutputFileNames[0] + ".devrescue.take" + to_string(i) + ".frames" + to_string(Input->F_Takes_Start) + "-.dv").c_str(),
                        (Merge_OutputFileNames[0] + ".devrescue.take" + to_string(i) + ".frames" + to_string(Input->F_Takes_Start) + '-' + to_string(LastBadFrame) + ".dv").c_str());
                    Input->F_Takes = nullptr;
                    Input->F_Takes_Start = -1;
                }
            }

            return true;
        }
    }

    // Store bad frames
    if (FirstFirstBadFrame != -1 && Frame_Pos <= LastBadFrame && (Merge_OutputFileNames[0] != "-"))
    {
        auto& Input = Inputs[Input_Rewind_Pos];
        auto& Frames = Input->Segments[Segment_Pos].Frames;
        auto& Frame = Frames[Frame_Pos];
        if (!Frame.Status[Status_FrameMissing] && Frame.Buffer.Data)
        {
            if (!Input->F_Takes)
            {
                Input->F_Takes_Start = Frame_Pos;
                Input->F_Takes = fopen((Merge_OutputFileNames[0] + ".devrescue.take" + to_string(Input_Rewind_Pos) + ".frames" + to_string(Input->F_Takes_Start) + "-.dv").c_str(), "wb");
            }
            if (Input->F_Takes)
            {
                fwrite(Frame.Buffer.Data, Frame.BlockStatus_Count * 80, 1, Input->F_Takes);
            }
        }
    }

    bool Frame_Speed = false;
    if (!((Speed && Speed != 1.0) || GetDvSpeedIsNormalPlayback(DvSpeed)))
    {
        Frame_Speed = true;
        if (OutputFrames_Speeds.empty() || !OutputFrames_Speeds[0])
            Output.Count_Frames_Ignored_Speed++;
    }

    bool Frame_Concealed = false;
    if (!(Prefered_Frame != -1 && !Inputs[Prefered_Frame]->Segments[Segment_Pos].Frames[Frame_Pos].FullConcealed))
    {
        Frame_Concealed = true;
        if (OutputFrames_Concealeds.empty() || !OutputFrames_Concealeds[0])
            Output.Count_Frames_Ignored_Concealed++;
    }

    if (true
        && (OutputFrames_NoData || (Prefered_Frame != -1 && !Inputs[Prefered_Frame]->Segments[Segment_Pos].Frames[Frame_Pos].NoData))
        && (ShowFrames_Intermediate || (Prefered_Frame != -1 && FirstBadFrame == -1))
        )
    {
        if (Verbosity > 5 && !(Verbosity <= 7 && !(!IsMissing && !IsOK)))
        {
            *Log << Log_Line.str() << endl;
        }
        if (Prefered_Frame != -1 && FirstBadFrame == -1) // Write only if there is some content from this specific frame
        {
            auto Write_Size = BlockStatus_Count * 80;
            if ((!Frame_Concealed || (!OutputFrames_Concealeds.empty() && OutputFrames_Concealeds[0])) &&
                (!Frame_Speed || (!OutputFrames_Speeds.empty() && OutputFrames_Speeds[0])))
            {
                fwrite(Output.OutputBuffer, Write_Size, 1, Output.F);
                Output.F_Pos += Write_Size;
            }

            for (size_t Pos = 0; Pos < Output.Copies.size(); Pos++)
            {
                if ((!Frame_Concealed || (OutputFrames_Concealeds.size() > Pos+1 && OutputFrames_Concealeds[Pos+1])) &&
                    (!Frame_Speed || (OutputFrames_Speeds.size() > Pos+1 && OutputFrames_Speeds[Pos+1])))
                {
                    fwrite(Output.OutputBuffer, Write_Size, 1, Output.Copies[Pos]);
                }
            }

            // Release memory
            for (size_t i = 0; i < Inputs.size(); i++)
            {
                auto& Input = Inputs[i];
                if (Input->DoNotUseFile)
                    continue;
                if (Segment_Pos >= Input->Segments.size())
                    continue;
                auto& Frames = Input->Segments[Segment_Pos].Frames;
                if (Frame_Pos >= Frames.size())
                    continue;
                auto& Frame = Frames[Frame_Pos];
                if (Frame.Buffer.Data)
                {
                    delete[] Frame.Buffer.Data;
                    Frame.Buffer.Data = nullptr;
                    Frame.Buffer.Size = 0;
                }
            }
        }
    }

    Frame_Pos++;
    return false;
}

//---------------------------------------------------------------------------
void dv_merge_private::AddFrameAnalysis(size_t InputPos, const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData, float Speed)
{
    if (Input_Rewind_Pos && LastBadFrame != -1)
    {
        if (Frame_Pos > LastBadFrame + 1)
        {
            Input_Rewind_Pos = 0;
            FirstFirstBadFrame = -1;
            FirstBadFrame = -1;
            LastBadFrame = -1;
            const auto Input_Count = Inputs.size();
            for (size_t i = 1; i < Input_Count; i++)
            {
                auto& Input = Inputs[i];
                Input->DoNotUseFile = true;
            }
        }
        InputPos = Input_Rewind_Pos;
    }

    // Coherency check
    lock_guard<mutex> Lock(Mutex);

    // Pre-processing
    if (Init())
        return;
    if (ManageRepeatedFrame(InputPos, FrameData))
        return;
    if (AppendFrameToList(InputPos, FrameData))
        return;
    if (ManagePartialFrame(InputPos, FrameData))
        return;
    if (TcSyncStart())
        return;

    // Processing
    while (!Process(Speed));

    if (Input_Rewind_Pos && LastBadFrame != -1)
    {
        if (Frame_Pos > LastBadFrame + 1)
        {
            for (size_t i = 0; i < Inputs.size(); i++)
            {
                auto& Input = Inputs[i];
                if (Input->F_Takes)
                {
                    fclose(Input->F_Takes);
                    remove((Merge_OutputFileNames[0] + ".devrescue.take" + to_string(i) + ".frames" + to_string(Input->F_Takes_Start) + '-' + to_string(LastBadFrame) + ".dv").c_str());
                    rename((Merge_OutputFileNames[0] + ".devrescue.take" + to_string(i) + ".frames" + to_string(Input->F_Takes_Start) + "-.dv").c_str(),
                           (Merge_OutputFileNames[0] + ".devrescue.take" + to_string(i) + ".frames" + to_string(Input->F_Takes_Start) + '-' + to_string(LastBadFrame) + ".dv").c_str());
                    Input->F_Takes = nullptr;
                    Input->F_Takes_Start = -1;
                }
            }
            Input_Rewind_Pos = 0;
            FirstFirstBadFrame = -1;
            FirstBadFrame = -1;
            LastBadFrame = -1;
            const auto Input_Count = Inputs.size();
            for (size_t i = 1; i < Input_Count; i++)
            {
                auto& Input = Inputs[i];
                Input->DoNotUseFile = true;
            }
        }
        InputPos = Input_Rewind_Pos;
    }
}

//---------------------------------------------------------------------------
void dv_merge_private::Finish()
{
    // Coherency check
    lock_guard<mutex> Lock(Mutex);
    if (Merge_Out.empty() || Merge_InputFileNames.size() > Inputs.size())
        return;

    if (Verbosity > 0 && Verbosity <= 7)
        UpdateDynamicDisplay();

    // Pre-processing
    if (SyncEnd(true))
        return;

    // Processing
    if (Count_Last_OK_Frames)
    {
        if (Verbosity > 5 && Verbosity <= 7 && Count_Last_OK_Frames)
        {
            if (Count_Last_OK_Frames > 1)
            {
                CurrentLine << setw(Inputs.size() + 2) << ' ' << '(' << setw(Formating_FrameCount_Width) << Count_Last_OK_Frames << " frames)";
            }
            *Log << CurrentLine.str() << endl;
            CurrentLine.str(string());
        }
        Count_Last_OK_Frames = 0;
    }
    while (!Process(0));

    // Post-processing
    if (Stats())
        return;
}

//---------------------------------------------------------------------------
bool dv_merge_private::Stats()
{
    // Stats - Frame count
    if (!Verbosity || MergeInfo_Format)
        return false;
    auto Input_Count = Inputs.size();
    auto Count_Blocks_Total = dv_merge_private::Count_Blocks.Total();
    auto Count_Frames_Total = dv_merge_private::Count_Frames.Total();
    *Log << '\n' << setfill(' ') << setw(Formating_FrameCount_Width) << Count_Frames_Total << " frames in total.\n\n";

    // Stats - Merge
    *Log << "How frames are merged:\n";
    ShowFrames(Count_Frames.OK, Count_Frames_Total, " are recovered from full frames.\n");
    ShowFrames(Count_Frames.Recovered, Count_Frames_Total, " are fully recovered based on reconstruction from blocks.\n");
    ShowFrames(Count_Frames.NOK, Count_Frames_Total, " remain with errors.\n");
    if (Count_Frames.NOK)
    {
        size_t Count_Frames_NotHealthy_Blocks_NotHealthy_Max = -1;
        size_t Count_Frames_NotHealthy_Blocks_NotHealthy_Max_Pos;
        for (size_t i = 0; i < Input_Count; i++)
        {
            if (Count_Frames_NotHealthy_Blocks_NotHealthy_Max > Inputs[i]->Count_Blocks_Bad_Frames_NOK())
            {
                Count_Frames_NotHealthy_Blocks_NotHealthy_Max = Inputs[i]->Count_Blocks_Bad_Frames_NOK();
                Count_Frames_NotHealthy_Blocks_NotHealthy_Max_Pos = i;
            }
        }
        if (Count_Frames_NotHealthy_Blocks_NotHealthy_Max != Count_Blocks.NOK)
        {
            *Log << "                     (in these frames, errors reduced from ";
                ShowBlocks(Count_Frames_NotHealthy_Blocks_NotHealthy_Max, Count_Frames_NOK_Blocks_Total, " bad in file ");
            *Log << Count_Frames_NotHealthy_Blocks_NotHealthy_Max_Pos << "\n                                                        to ";
            ShowBlocks(Count_Blocks.NOK, Count_Frames_NOK_Blocks_Total, " bad).\n");
        }
    }
    ShowFrames(Count_Frames.Missing, Count_Frames_Total, " are still missing.\n");
    *Log << '\n';

    // Stats - What is used
    *Log << "Usage of input files:\n";
    for (size_t i = 0; i < Input_Count; i++)
    {
        if (Inputs[i]->Count_Blocks_Used)
        {
            ShowBlocks(Inputs[i]->Count_Blocks_Used, Count_Blocks_Total);
            *Log << " from file " << i << "     used.";
            ShowFrames(Inputs[i]->Count_Frames_Repeated, Count_Frames_Total, " were repetition and discarded.");
        }
        else
        {
            for (size_t j = 0; j < Formating_BlockCount_Width + Formating_Precision + 22; j++)
                *Log << ' ';
            *Log << "file " << i << " not used.";
        }
        *Log << '\n';
    }
    *Log << '\n';

    // Stats - Per file
    float Frames_Bad_MinRatio = 1;
    float Blocks_Bad_MinRatio = 1;
    *Log << "Input files summary:\n";
    for (size_t i = 0; i < Input_Count; i++)
    {
        auto Blocks_Bad = Inputs[i]->Count_Blocks_Bad();
        auto Frames_Bad = Inputs[i]->Count_Frames_Bad();
        auto Blocks_Bad_Ratio = (float)Blocks_Bad / Count_Blocks_Total;
        auto Frames_Bad_Ratio = (float)Frames_Bad / Count_Frames_Total;
        if (!CanRewind || !i)
        {
            if (Blocks_Bad_MinRatio > Blocks_Bad_Ratio)
                Blocks_Bad_MinRatio = Blocks_Bad_Ratio;
            if ((!CanRewind || !i) && Frames_Bad_MinRatio > Frames_Bad_Ratio)
                Frames_Bad_MinRatio = Frames_Bad_Ratio;
            if (!CanRewind || !i)
                *Log << "File " << i << ": ";
        }
        else
            *Log << "Rew  " << i << ": ";
        if (Blocks_Bad)
        {
            ShowBlocks(Blocks_Bad, Count_Blocks_Total, " have errors, in ");
            ShowFrames(Frames_Bad, Count_Frames_Total, "\n", true);
        }
        else
        {
            *Log << "No error detected\n";
        }
    }
    *Log << '\n';

    // Stats - Result
    auto Blocks_Bad = Count_Blocks.Bad();
    auto Blocks_Bad_FinalRatio = (float)Blocks_Bad / Count_Blocks_Total;
    auto Frames_Bad = Count_Frames.Bad();
    auto Frames_Bad_FinalRatio = (float)Frames_Bad / Count_Frames_Total;
    *Log << "Result:\n";
    *Log << std::setw(8) << ' ';
    ShowBlocks(Blocks_Bad, Count_Blocks_Total, " have errors, in ");
    ShowFrames(Frames_Bad, Count_Frames_Total, "\n", true);
    if (Frames_Bad_MinRatio != Frames_Bad_FinalRatio || Blocks_Bad_MinRatio != Blocks_Bad_FinalRatio)
    {
        *Log << std::setw(Formating_BlockCount_Width + 9) << ' ';
        if (Blocks_Bad_MinRatio != Blocks_Bad_FinalRatio)
        {
            *Log << "(-"
                << setfill(' ') << fixed << setw(Formating_Precision + 2) << setprecision(Formating_Precision - 2) << (Blocks_Bad_MinRatio - Blocks_Bad_FinalRatio) * 100
                << ')';
        }
        else
        {
            *Log << std::setw(Formating_Precision + 5) << ' ';
        }
        if (Frames_Bad_MinRatio != Frames_Bad_FinalRatio)
        {
            *Log << std::setw(Formating_FrameCount_Width + 28) << ' ';
            *Log << "(-"
                << setfill(' ') << fixed << setw(Formating_Precision + 2) << setprecision(Formating_Precision - 2) << (Frames_Bad_MinRatio - Frames_Bad_FinalRatio) * 100
                << ')';
        }
        *Log << '\n';
    }
    if (Output.Count_Frames_Ignored_Speed || Output.Count_Frames_Ignored_Concealed)
    {
        *Log << '\n';
        *Log << "Frames discarded in output:";
        *Log << std::setw(Formating_BlockCount_Width + 2) << ' ';
        ShowFrames(Output.Count_Frames_Ignored_Speed, Count_Frames_Total, " discarded for non-standard playback speed.");
        *Log << '\n';
        *Log << std::setw(Formating_BlockCount_Width + 2) << ' ';
        ShowFrames(Output.Count_Frames_Ignored_Concealed, Count_Frames_Total, " discarded for full concealment.");
        *Log << '\n';
    }
    *Log << flush;

    if (Output.F_Pos)
        Merge_Out_Size = Output.F_Pos;

    return false;
}

//---------------------------------------------------------------------------
void dv_merge_private::AddFrameData(size_t InputPos, const uint8_t* Buffer, size_t Buffer_Size)
{
    // Coherency check
    lock_guard<mutex> Lock(Mutex);

    // Pre-processing
    if (Init())
        return;

    // Add frame
    auto& Input = Inputs[InputPos + Input_Rewind_Pos];
    if (!Input->DV_Data)
        Input->DV_Data = new dv_data;
    Input->DV_Data->push_back(Buffer, Buffer_Size);
}