/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/Output.h"
#include "Common/Output_Captions_Scc.h"
#include "Common/ProcessFile.h"
#include "TimeCode.h"
#include <cmath>
#include <algorithm>
#include <fstream>
//---------------------------------------------------------------------------

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
static const char* const Writer_Name = "Captions SCC";

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
const char Hex2String[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

//---------------------------------------------------------------------------
static void InjectBeforeExtension(string& Name, const char* ToInject, size_t Index)
{
    auto DotPos = Name.rfind('.');
    if (DotPos == string::npos)
        DotPos = Name.size();
    Name.insert(DotPos, ToInject + to_string(Index));
}

//***************************************************************************
// Output
//***************************************************************************

//---------------------------------------------------------------------------
static return_value Output_Captions_Scc(const string& OutName, const TimeCode& TC_Base, const vector<file::captions_fielddata>& PerFrame_Captions, const vector<size_t>& IgnoredFrames, ostream* Err)
{
    auto ToReturn = ReturnValue_OK;

    // Open file
    string Text;
    if (Verbosity == 10)
        *Err << "Debug: opening (out, trunc) \"" << OutName << "\"..." << endl;
    ofstream Out(OutName, ios_base::trunc);
    if (Verbosity == 10)
        *Err << "Debug: opening (out, trunc) \"" << OutName << "\"... Done." << endl;
    if (!Out.is_open())
    {
        if (Err)
            *Err << "Error: can not open " << OutName << " for writing.\n";
        return ReturnValue_ERROR;
    }

    // File header
    Text += "Scenarist_SCC V1.0\n\n";

    // By Frame - For each line
    for (const auto& Frame : PerFrame_Captions)
    {
        if (find(IgnoredFrames.begin(), IgnoredFrames.end(), Frame.StartFrameNumber) != IgnoredFrames.end())
            continue;

        TimeCode TC = TC_Base + Frame.StartFrameNumber;
        Text += TC.ToString();

        for (const auto& Caption : Frame.Captions)
        {
            char DataString[5];
            DataString[0] = ' ';
            DataString[1] = Hex2String[Caption.Data[0] >> 4];
            DataString[2] = Hex2String[Caption.Data[0] & 0xF];
            DataString[3] = Hex2String[Caption.Data[1] >> 4];
            DataString[4] = Hex2String[Caption.Data[1] & 0xF];
            Text.append(DataString, 5);
        }

        Text.append(2, '\n');

        // Write content to output
        if (Out.is_open() && !WriteIfBig(Out, Text, Err, Writer_Name))
            ToReturn = ReturnValue_ERROR;
    }

    // Write content to output
    if (Out.is_open() && !Write(Out, Text, Err, Writer_Name))
        ToReturn = ReturnValue_ERROR;

    return ToReturn;
}

//---------------------------------------------------------------------------
return_value Output_Captions_Scc(const Core::OutFile& Out, const TimeCode* OffsetTimeCode, std::vector<file*>& PerFile, ostream* Err)
{
    auto ToReturn = ReturnValue_OK;

    bool OutputFrames_Speed = true;
    bool OutputFrames_Concealed = true;

    auto It = find(Merge_OutputFileNames.begin(), Merge_OutputFileNames.end(), Out.Merge_OutputFileName);
    if (It != Merge_OutputFileNames.end())
    {
        size_t Pos = It - Merge_OutputFileNames.begin();
        OutputFrames_Speed = OutputFrames_Speeds[Pos];
        OutputFrames_Concealed = OutputFrames_Concealeds[Pos];
    }

    for (const auto& File : PerFile)
    {
        if (File->PerFrame_Captions_PerSeq_PerField.empty())
            continue; // Show the file only if there is some captions content

        // filter
        vector<size_t> IgnoredFrames;
        if (!OutputFrames_Speed || !OutputFrames_Concealed)
        {
            for (auto Frame = File->PerFrame.begin(); Frame < File->PerFrame.end(); ++Frame)
            {
                coherency_flags Coherency(*Frame);
                if (!OutputFrames_Concealed && Coherency.full_conceal())
                {
                    IgnoredFrames.push_back(Frame - File->PerFrame.begin());
                    continue;
                }

                if (!OutputFrames_Speed && !GetDvSpeedIsNormalPlayback(GetDvSpeed(**Frame)))
                {
                    IgnoredFrames.push_back(Frame - File->PerFrame.begin());
                    continue;
                }
            }
        }

        // Init time code
        TimeCode TC_Base;
        if (OffsetTimeCode)
            TC_Base = *OffsetTimeCode;
        else
            TC_Base.FromFrames(0);
        TC_Base.FramesPerSecond = (uint8_t)lround(File->FrameRate);
        TC_Base.FramesPerSecond_Is1001 = TC_Base.FramesPerSecond != File->FrameRate;
        if (!OffsetTimeCode || !OffsetTimeCode->IsValid())
            TC_Base.DropFrame = TC_Base.FramesPerSecond_Is1001;
        if (OffsetTimeCode && !TC_Base.IsValid())
        {
            // Try to use DV time code
            auto FromDvTimecode = timecode(*File->PerFrame.begin());
            if (FromDvTimecode.HasValue())
            {
                auto FromDvTimecodeSeconds = FromDvTimecode.TimeInSeconds();
                TC_Base.Hours = FromDvTimecodeSeconds / 3600; FromDvTimecodeSeconds %= 3600;
                TC_Base.Minutes = FromDvTimecodeSeconds / 60; FromDvTimecodeSeconds %= 60;
                TC_Base.Seconds = FromDvTimecodeSeconds;
                TC_Base.Frames = FromDvTimecode.Frames();
                TC_Base.DropFrame = FromDvTimecode.DropFrame();
            }
        }

        // Per Dseq
        for (size_t i = 0; i < File->PerFrame_Captions_PerSeq_PerField.size(); i++) // Per Dseq
        {
            for (size_t j = 0; j < 2; j++) // Per field
            {
                string OutNameWithDseq(Out.Name);
                if (File->PerFrame_Captions_PerSeq_PerField.size() > 1)
                    InjectBeforeExtension(OutNameWithDseq, ".dseq", i);
                if (!File->PerFrame_Captions_PerSeq_PerField[i].FieldData[0].empty() && !File->PerFrame_Captions_PerSeq_PerField[i].FieldData[1].empty())
                    InjectBeforeExtension(OutNameWithDseq, ".field", i + 1);

                if (!File->PerFrame_Captions_PerSeq_PerField[i].FieldData[j].empty() && !Output_Captions_Scc(OutNameWithDseq, TC_Base, File->PerFrame_Captions_PerSeq_PerField[i].FieldData[j], IgnoredFrames, Err))
                    ToReturn = ReturnValue_ERROR;
            }
        }
    }

    return ToReturn;
}
