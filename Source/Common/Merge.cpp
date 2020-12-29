/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/ProcessFile.h"
#include "ZenLib/Ztring.h"
#include "Output.h"
#include <iostream>
#include <mutex>
#include <iomanip>
#include <map>
#include <bitset>
#include "TimeCode.h"
#include "CLI/CLI_Help.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
vector<string> Merge_InputFileNames;
string Merge_OutputFileName;
uint8_t Verbosity = 5;
//---------------------------------------------------------------------------

namespace
{
    void Merge_Help()
    {
        if (Verbosity)
            cout << NameVersion_Text() << '\n';
        if (Verbosity <= 5)
            return;
        cout <<
            "\n"
            "# TC Used Status Comments\n"
            "#:        the frame number (0-based)\n"
            "SS;FF:      the time code, if unique accross all files, SS;FF format\n"
            "S:        the file number (0-based) selected as the reference (non A/V blocks are taken from it), \"X\" if from several frames\n"
            "\n"
            "Status:   the status of the frame, 1 char per file\n"
            "              = Fine\n"
            "            T = Time code is missing\n"
            "            P = Frame has problem\n"
            "            M = Frame is missing\n"
            "\n"
            "Comments: comments about the issues found\n"
            "              --> Nothing displayed\n"
            "            T --> List of time codes, \"??:??\" if no time code found\n"
            "            P --> List of count of blocks picked per file, 1 number per file + count of blocks remaining with errors\n"
            "            M --> Nothing displayed\n"
            << endl;
    }

    string TC_String(TimeCode const& TC)
    {
        if (!TC.HasValue())
            return "XX:XX";

        auto ToReturn = TC.ToString();
        if (ToReturn.size() > 6)
            ToReturn.erase(0, 6);
        else
            ToReturn = "     ";

        return ToReturn;
    }

    mutex Mutex;

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

    struct per_frame
    {
        bitset<Status_Max>  Status;
        TimeCode            TC;
        uint8_t*            BlockStatus = nullptr;
        size_t              BlockStatus_Count = 0;

        per_frame() = default;
        per_frame(status const& Status_, ::TimeCode const& TC_, uint8_t* const& BlockStatus_, size_t BlockStatus_Count_) :
            Status((unsigned long)(1 << (unsigned)Status_)), TC(TC_), BlockStatus(BlockStatus_), BlockStatus_Count(BlockStatus_Count_) {}
    };

    struct per_file
    {
        FILE*               F = nullptr;
        uint8_t             Buffer[144000 * 4];
        size_t              Count_Blocks[BlockStatus_Max] = {};
        size_t              Count_Blocks_NOK_Frames_NOK = 0;
        size_t              Count_Blocks_Missing = 0;
        size_t              Count_Blocks_Missing_Frames_NOK = 0;
        size_t              Count_Blocks_Used = 0;
        size_t              Count_Frames_OK = 0;
        size_t              Count_Frames_NOK = 0;
        size_t              Count_Frames_Missing = 0;
        bool                DoNotUseFile = false;
        vector<per_frame>   Frames;

        ~per_file()
        {
            if (F)
                fclose(F);
            for (auto& Frame : Frames)
            {
                delete[] Frame.BlockStatus;
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
    vector<per_file> Inputs;
    per_file Output;

    // Stats
    size_t Count_Blocks_OK = 0;
    size_t Count_Blocks_NOK = 0;
    size_t Count_Blocks_Missing = 0;
    size_t Count_Blocks_Bad()
    {
        return Count_Blocks_NOK + Count_Blocks_Missing;
    }
    size_t Count_Blocks_Total()
    {
        return Count_Blocks_OK + Count_Blocks_NOK + Count_Blocks_Missing;
    }
    size_t Count_Frames_NOK_Blocks_Total = 0;
    size_t Count_Frame_Pos = 0;
    size_t Count_Frames_OK = 0;
    size_t Count_Frames_Recovered = 0;
    size_t Count_Frames_NOK = 0;
    size_t Count_Frames_Missing = 0;
    size_t Count_Frames_Bad()
    {
        return Count_Frames_NOK + Count_Frames_Missing;
    }
    size_t Count_Frames_Total()
    {
        return Count_Frames_OK + Count_Frames_NOK + Count_Frames_Missing;
    }
    size_t Frames_Status_Max = -1;

    // Display
    const int Formating_FrameCount_Width = 6;
    const int Formating_BlockCount_Width = 9;
    const int Formating_FrameBlockCount_Width = 4;
    const int Formating_Precision = 6;
}

//---------------------------------------------------------------------------
void file::Merge_AddFrame(const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData)
{
    Mutex.lock();
    auto FileCount = Merge_InputFileNames.size();
    if (Inputs.empty())
    {
        Merge_Help();

        Inputs.resize(FileCount);
        for (auto const& Inputs_FileName : Merge_InputFileNames)
        {
            if (Verbosity > 5)
                cout << "File " << &Inputs_FileName - &Merge_InputFileNames.front() << ": " << Inputs_FileName << '\n';
            Inputs[&Inputs_FileName - &Merge_InputFileNames.front()].F = fopen(Inputs_FileName.c_str(), "rb");
        }
        Output.F = fopen(Merge_OutputFileName.c_str(), "wb");

        if (Verbosity > 5)
            cout << '\n' << std::setw(Formating_FrameCount_Width - 1) << ' ' << "#|SS:FF|U|St|Comments" << endl;
    }

    // Stop if frame repetition was found
    auto& Input = Inputs[Merge_FilePos];
    if (Input.DoNotUseFile || timecode(FrameData->TimeCode).Repeat())
    {
        if (!Input.DoNotUseFile)
        {
            Input.DoNotUseFile = true;
        }
        Mutex.unlock();
        return;
    }

    per_frame CurrentFrame;
    auto BlockStatus_Count = FrameData->BlockStatus_Count;

    // Time code jumps - after first frame
    timecode TC_Temp(FrameData->TimeCode);
    if (TC_Temp.HasValue())
        CurrentFrame.TC = TimeCode(TC_Temp.TimeInSeconds() / 3600, (TC_Temp.TimeInSeconds() / 60) % 60, TC_Temp.TimeInSeconds() % 60, TC_Temp.Frames(), 30 /*TEMP*/, TC_Temp.DropFrame());
    if (!Input.Frames.empty())
    {
        TimeCode TC_Previous(Input.Frames.back().TC);
        TC_Previous++;
        if (CurrentFrame.TC.HasValue())
        {
            while (TC_Previous != CurrentFrame.TC)
            {
                Input.Frames.emplace_back(Status_FrameMissing, TC_Previous, nullptr, BlockStatus_Count);
                Input.Count_Blocks_Missing += BlockStatus_Count;
                Input.Count_Frames_Missing++;
                TC_Previous++;
            }
        }
        else
        {
            CurrentFrame.TC = TC_Previous;
            CurrentFrame.Status.set(Status_TimeCodeIssue);
        }
    }

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
        Input.Count_Frames_NOK++;
    }
    else
    {
        Input.Count_Frames_OK++;
    }
    for (int i = 0; i < BlockStatus_Max; i++)
    {
        Input.Count_Blocks[i] += Count_Blocks[i];
    }

    Input.Frames.emplace_back(move(CurrentFrame));

    // Time code jumps - first frame
    if (!Count_Frame_Pos)
    {
        Frames_Status_Max = -1;
        for (size_t i = 0; i < FileCount; i++)
            if (Frames_Status_Max > Inputs[i].Frames.size())
                Frames_Status_Max = Inputs[i].Frames.size();
        if (Frames_Status_Max)
        {
            TimeCode TC_Min = Inputs[0].Frames[0].TC;
            TimeCode TC_Max = TC_Min;
            for (size_t i = 0; i < FileCount; i++)
            {
                if (TC_Min.ToFrames() > Inputs[i].Frames[0].TC.ToFrames())
                    TC_Min = Inputs[i].Frames[0].TC;
                if (TC_Max.ToFrames() < Inputs[i].Frames[0].TC.ToFrames())
                    TC_Max = Inputs[i].Frames[0].TC;
            }
            if (TC_Min != TC_Max)
                for (size_t i = 0; i < FileCount; i++)
                {
                    auto TC_Previous = TC_Min;
                    auto TC_End = Inputs[i].Frames.front().TC;
                    size_t Offset = 0;
                    while (TC_Previous != TC_End)
                    {
                        per_frame PreviousFrame;
                        PreviousFrame.TC = TC_Previous;
                        PreviousFrame.Status.set(Status_FrameMissing);
                        Inputs[i].Frames.emplace(Inputs[i].Frames.begin() + Offset, move(PreviousFrame));
                        TC_Previous++;
                        Offset++;
                    }
                }
        }
    }

    // Check which frames are available everywhere
    Frames_Status_Max = -1;
    for (size_t i = 0; i < FileCount; i++)
        if (Frames_Status_Max > Inputs[i].Frames.size())
            Frames_Status_Max = Inputs[i].Frames.size();
    Merge_Process();
    Mutex.unlock();
}

//---------------------------------------------------------------------------
void file::Merge_Process()
{
    auto FileCount = Merge_InputFileNames.size();

    for (; Count_Frame_Pos < Frames_Status_Max; Count_Frame_Pos++)
    {
        if (Verbosity > 5)
            cout << setfill(' ') << setw(Formating_FrameCount_Width) << Count_Frame_Pos;

        size_t BlockStatus_Count = 0;
        for (size_t i = 0; i < FileCount; i++)
            if (BlockStatus_Count < Inputs[i].Frames[Count_Frame_Pos].BlockStatus_Count)
                BlockStatus_Count = Inputs[i].Frames[Count_Frame_Pos].BlockStatus_Count;

        // Copy 1 frame in memory, for each input
        for (size_t i = 0; i < FileCount; i++)
        {
            if (!Inputs[i].Frames[Count_Frame_Pos].Status[Status_FrameMissing])
            {
                auto B = fread(Inputs[i].Buffer, 1, Inputs[i].Frames[Count_Frame_Pos].BlockStatus_Count * 80, Inputs[i].F);
                if (B != Inputs[i].Frames[Count_Frame_Pos].BlockStatus_Count * 80)
                    int A = 0;
            }
        }

        // Find a prefered frame
        size_t Prefered_Frame = -1;
        size_t Prefered_TC = -1;
        for (size_t i = 0; i < FileCount; i++)
        {
            auto& Input = Inputs[i];
            if (Prefered_Frame == -1 && !Input.Frames[Count_Frame_Pos].Status[Status_FrameMissing] && !Input.Frames[Count_Frame_Pos].Status[Status_BlockIssue])
                Prefered_Frame = i;
            if (Prefered_TC == -1 && !Input.Frames[Count_Frame_Pos].Status[Status_TimeCodeIssue])
                Prefered_TC = i;
        }

        if (Verbosity > 5)
        {
            if (Prefered_TC != -1)
                cout << ' ' << TC_String(Inputs[Prefered_TC].Frames[Count_Frame_Pos].TC);
            else
                cout << " ??:??";
            if (Prefered_Frame != -1)
                cout << ' ' << Prefered_Frame;
            else
                cout << " X";
            cout << ' ';
            for (size_t i = 0; i < FileCount; i++)
            {
                if (Inputs[i].Frames[Count_Frame_Pos].Status[Status_FrameMissing])
                    cout << 'M';
                else if (Inputs[i].Frames[Count_Frame_Pos].Status[Status_BlockIssue])
                    cout << 'P';
                else if (Inputs[i].Frames[Count_Frame_Pos].Status[Status_TimeCodeIssue])
                    cout << 'T';
                else
                    cout << ' ';
            }
        }

        // Find valid blocks
        if (Prefered_Frame != -1)
        {
            Inputs[Prefered_Frame].Count_Blocks_Used += BlockStatus_Count;
            Count_Blocks_OK += BlockStatus_Count;
            Count_Frames_OK++;
            memcpy(Output.Buffer, Inputs[Prefered_Frame].Buffer, Inputs[Prefered_Frame].Frames[Count_Frame_Pos].BlockStatus_Count * 80);
        }
        else
        {
            size_t ThisFrame_Count_Blocks_Used[100];
            memset(ThisFrame_Count_Blocks_Used, 0, FileCount * sizeof(size_t));
            size_t IssueCount = 0;

            // For each block
            vector<size_t> Priorities; // Find the frame with the minimum count of bad blocks
            Priorities.resize(FileCount);
            for (int b = 0; b < BlockStatus_Count; b++)
            {
                for (int f = 0; f < FileCount; f++)
                    if (Inputs[f].Frames[Count_Frame_Pos].Status[Status_BlockIssue])
                    {
                        switch (Inputs[f].Frames[Count_Frame_Pos].BlockStatus[b])
                        {
                        case BlockStatus_NOK:
                            Priorities[f]++;
                            Inputs[f].Count_Blocks_NOK_Frames_NOK++;
                            break;
                        default:;
                        }
                    }
                    else if (!b)
                        Priorities[f] = (size_t)-1;
            }
            map<size_t, vector<size_t>> Priorities2;
            for (int f = 0; f < FileCount; f++)
                Priorities2[Priorities[f]].push_back(f);
            Priorities.clear();
            for (auto p : Priorities2)
                for (auto k : p.second)
                    Priorities.push_back(k);
            if (Priorities2.begin()->first == (size_t)-1)
            {
                Count_Blocks_Missing += BlockStatus_Count;
                Count_Frames_Missing++; // Don't try to find good blocks if there is no file with good blocks
            }
            else
            {
                for (size_t i = 0; i < FileCount; i++)
                {
                    if (Inputs[i].Frames[Count_Frame_Pos].Status[Status_FrameMissing])
                        Inputs[i].Count_Blocks_Missing_Frames_NOK += BlockStatus_Count;
                }
                Prefered_Frame = Priorities[0];
                Inputs[Prefered_Frame].Count_Blocks_Used += BlockStatus_Count;
                memcpy(Output.Buffer, Inputs[Priorities[0]].Buffer, BlockStatus_Count * 80); // Copy the content of the file having the less issues
                for (int b = 0; b < BlockStatus_Count; b++)
                {
                    bool NoIssue = false;
                    for (int f = 0; f < FileCount; f++)
                    {
                        auto p = Priorities[f];
                        if (Inputs[p].Frames[Count_Frame_Pos].Status[Status_BlockIssue])
                        {
                            switch (Inputs[p].Frames[Count_Frame_Pos].BlockStatus[b])
                            {
                            case 1:
                                if (!NoIssue)
                                {
                                    if (f)
                                        memcpy(Output.Buffer + b * 80, Inputs[p].Buffer + b * 80, 80);
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
                        IssueCount++;
                }
                if (Verbosity > 5)
                    cout << ',';
                for (int f = 0; f < FileCount; f++)
                {
                    if (f != Prefered_Frame)
                    {
                        Inputs[f].Count_Blocks_Used += ThisFrame_Count_Blocks_Used[f];
                        Inputs[Prefered_Frame].Count_Blocks_Used -= ThisFrame_Count_Blocks_Used[f];
                    }
                    if (Verbosity > 5)
                        cout << ' ' << fixed << setw(Formating_FrameBlockCount_Width) << ThisFrame_Count_Blocks_Used[f];
                }
                if (Verbosity > 5)
                    cout << " block picks";
                if (IssueCount)
                {
                    if (Verbosity > 5)
                        cout << " & " << fixed << setw(Formating_FrameBlockCount_Width) << IssueCount << " remaining block errors";
                    Count_Frames_NOK++;
                    Count_Frames_NOK_Blocks_Total += BlockStatus_Count;
                }
                else
                {
                    Count_Frames_Recovered++;
                }
                Count_Blocks_OK += BlockStatus_Count - IssueCount;
                Count_Blocks_NOK += IssueCount;
            }
        }

        if (Verbosity > 5)
        {
            bool HasTimeCodeIssue = false;
            for (size_t i = 1; i < FileCount; i++)
                if (Inputs[i].Frames[Count_Frame_Pos].Status[Status_TimeCodeIssue])
                    HasTimeCodeIssue = true;
            if (HasTimeCodeIssue)
            {
                if (Verbosity > 5)
                {
                    cout << ',';
                    for (size_t i = 0; i < FileCount; i++)
                    {
                        cout << ' ';
                        cout << (Inputs[i].Frames[Count_Frame_Pos].Status[Status_TimeCodeIssue] ? "??:??" : TC_String(Inputs[i].Frames[Count_Frame_Pos].TC));
                    }
                }
            }
        }

        if (Prefered_Frame != -1) // Write only if there is some content from this specific frame
            fwrite(Output.Buffer, BlockStatus_Count * 80, 1, Output.F);
        if (Verbosity > 5)
            cout << endl;
    }
}

void ShowFrames(size_t Count, size_t Total, const char* Next = nullptr)
{
    if (!Count)
        return;
    auto Percent = (float)Count / Total * 100;
    cout << fixed << setw(Formating_FrameCount_Width) << Count << " = " << fixed << setw(Formating_Precision + 2) << setprecision(Formating_Precision - 2) << Percent << "% of frames";
    if (Next)
        cout << Next;
}
void ShowBlocks(size_t Count, size_t Total, const char* Next = nullptr)
{
    auto Percent = (float)Count / Total * 100;
    cout << fixed << setw(Formating_BlockCount_Width) << Count << " = " << fixed << setw(Formating_Precision + 2) << setprecision(Formating_Precision - 2) << Percent << "% of blocks";
    if (Next)
        cout << Next;
}
//---------------------------------------------------------------------------
void file::Merge_Finish()
{
    if (Merge_OutputFileName.empty())
        return;

    // Check which frames are available at least once
    auto FileCount = Merge_InputFileNames.size();
    size_t LongestFile = 0;
    for (size_t i = 0; i < FileCount; i++)
        if (Frames_Status_Max < Inputs[i].Frames.size())
        {
            Frames_Status_Max = Inputs[i].Frames.size();
            LongestFile = i;
        }
    auto BlockStatus_Count = Inputs[LongestFile].Frames.back().BlockStatus_Count;
    for (size_t i = 0; i < FileCount; i++)
    {
        auto& Input = Inputs[i];
        while (Input.Frames.size() < Frames_Status_Max)
        {

            Input.Frames.emplace_back(Status_FrameMissing, TimeCode(), nullptr, BlockStatus_Count);
            Input.Count_Blocks_Missing += BlockStatus_Count;
            Input.Count_Frames_Missing++;
        }
    }
    Merge_Process();

    // Stats - Frame count
    if (!Verbosity)
        return;
    auto Count_Blocks_Total = ::Count_Blocks_Total();
    FrameNumber = Frames_Status_Max;
    cout << '\n' << setfill(' ') << setw(Formating_FrameCount_Width) << FrameNumber << " frames in total.\n\n";

    // Stats - Merge
    cout << "How frames are merged:\n";
    ShowFrames(Count_Frames_OK, FrameNumber, " are recovered from full frames.\n");
    ShowFrames(Count_Frames_Recovered, FrameNumber, " are fully recovered based on reconstruction from blocks.\n");
    ShowFrames(Count_Frames_NOK, FrameNumber, " remain with errors.\n");
    if (Count_Frames_NOK)
    {
        size_t Count_Frames_NotHealthy_Blocks_NotHealthy_Max = -1;
        size_t Count_Frames_NotHealthy_Blocks_NotHealthy_Max_Pos;
        for (size_t i = 0; i < FileCount; i++)
        {
            if (Count_Frames_NotHealthy_Blocks_NotHealthy_Max > Inputs[i].Count_Blocks_Bad_Frames_NOK())
            {
                Count_Frames_NotHealthy_Blocks_NotHealthy_Max = Inputs[i].Count_Blocks_Bad_Frames_NOK();
                Count_Frames_NotHealthy_Blocks_NotHealthy_Max_Pos = i;
            }
        }
        if (Count_Frames_NotHealthy_Blocks_NotHealthy_Max != Count_Blocks_NOK)
        {
            cout << "                     (in these frames, errors reduced from ";
                ShowBlocks(Count_Frames_NotHealthy_Blocks_NotHealthy_Max, Count_Frames_NOK_Blocks_Total, " bad in file ");
            cout << Count_Frames_NotHealthy_Blocks_NotHealthy_Max_Pos << "\n                                                        to ";
            ShowBlocks(Count_Blocks_NOK, Count_Frames_NOK_Blocks_Total, " bad).\n");
        }
    }
    ShowFrames(Count_Frames_Missing, FrameNumber, " are still missing.\n");
    cout << '\n';

    // Stats - What is used
    cout << "Usage of input files:\n";
    for (size_t i = 0; i < FileCount; i++)
    {
        if (Inputs[i].Count_Blocks_Used)
        {
            ShowBlocks(Inputs[i].Count_Blocks_Used, Count_Blocks_Total);
            cout << " from file " << i << "     used.";
        }
        else
        {
            for (size_t j = 0; j < Formating_BlockCount_Width + Formating_Precision + 22; j++)
                cout << ' ';
            cout << "file " << i << " not used.";
        }
        cout << '\n';
    }
    cout << '\n';

    // Stats - Per file
    float Frames_Bad_MinRatio = 1;
    float Blocks_Bad_MinRatio = 1;
    cout << "Input files summary:\n";
    for (size_t i = 0; i < FileCount; i++)
    {
        auto Blocks_Bad = Inputs[i].Count_Blocks_Bad();
        auto Frames_Bad = Inputs[i].Count_Frames_Bad();
        auto Blocks_Bad_Ratio = (float)Blocks_Bad / Count_Blocks_Total;
        auto Frames_Bad_Ratio = (float)Frames_Bad / FrameNumber;
        if (Blocks_Bad_MinRatio > Blocks_Bad_Ratio)
            Blocks_Bad_MinRatio = Blocks_Bad_Ratio;
        if (Frames_Bad_MinRatio > Frames_Bad_Ratio)
            Frames_Bad_MinRatio = Frames_Bad_Ratio;
        cout << "File " << i << ": ";
        if (Blocks_Bad)
        {
            ShowBlocks(Blocks_Bad, Count_Blocks_Total, " have errors, in ");
            ShowFrames(Frames_Bad, FrameNumber, "\n");
        }
        else
        {
            cout << "No error detected\n";
        }
    }
    cout << '\n';

    // Stats - Result
    auto Blocks_Bad = Count_Blocks_Bad();
    auto Blocks_Bad_FinalRatio = (float)Blocks_Bad / Count_Blocks_Total;
    auto Frames_Bad = Count_Frames_Bad();
    auto Frames_Bad_FinalRatio = (float)Frames_Bad / FrameNumber;
    cout << "Result:\n";
    cout << std::setw(8) << ' ';
    ShowBlocks(Blocks_Bad, Count_Blocks_Total, " have errors, in ");
    ShowFrames(Frames_Bad, FrameNumber, "\n");
    if (Frames_Bad_MinRatio != Frames_Bad_FinalRatio || Blocks_Bad_MinRatio != Blocks_Bad_FinalRatio)
    {
        cout << std::setw(Formating_BlockCount_Width + 9) << ' ';
        if (Blocks_Bad_MinRatio != Blocks_Bad_FinalRatio)
        {
            cout << "(-"
                << setfill(' ') << fixed << setw(Formating_Precision + 2) << setprecision(Formating_Precision - 2) << (Blocks_Bad_MinRatio - Blocks_Bad_FinalRatio) * 100
                << ')';
        }
        else
        {
            cout << std::setw(Formating_Precision + 5) << ' ';
        }
        if (Frames_Bad_MinRatio != Frames_Bad_FinalRatio)
        {
            cout << std::setw(Formating_FrameCount_Width + 28) << ' ';
            cout << "(-"
                << setfill(' ') << fixed << setw(Formating_Precision + 2) << setprecision(Formating_Precision - 2) << (Frames_Bad_MinRatio - Frames_Bad_FinalRatio) * 100
                << ')';
        }
        cout << '\n';
    }
    cout << flush;
}
