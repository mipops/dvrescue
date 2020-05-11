/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/Output.h"
#include "Common/Output_Xml.h"
#include "Common/ProcessFile.h"
#include "ZenLib/Ztring.h"
#include <utility>
using namespace ZenLib;
//---------------------------------------------------------------------------

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
static const char* const Writer_Name = "WebVTT";

//***************************************************************************
// Helpers
//***************************************************************************

static void Xml_Sta_Element(string& Text, int Sta, size_t n, size_t n_even = size_t(-1))
{
    if (!n)
        return;

    Text += "STA t=";
    Text += to_string(Sta);
    Text += " n=";
    Text += to_string(n);
    if (n_even != size_t(-1))
    {
        Text += " n_even=";
        Text += to_string(n_even);
    }
    Text += ", ";
}

static void Xml_Sta_Elements(string& Text, const size_t* const Stas, const size_t* const Stas_even = nullptr)
{
    for (auto Sta = 0; Sta < Sta_Size; Sta++)
    {
        auto n = Stas[Sta];
        auto n_even = Stas_even == nullptr ? size_t(-1) : Stas_even[Sta];
        Xml_Sta_Element(Text, Sta, n, n_even);
    }
}

static void Xml_Aud_Element(string& Text, size_t o, size_t n, size_t n_even = size_t(-1))
{
    if (!n)
        return;

    Text += "AUD n=";
    Text += to_string(n);
    if (n_even != size_t(-1))
    {
        Text += " n_even=";
        Text += to_string(n_even);
    }
    Text += ", ";
}

//***************************************************************************
// Output
//***************************************************************************

//---------------------------------------------------------------------------
return_value Output_Webvtt(ostream& Out, std::vector<file*>& PerFile, ostream* Err = nullptr)
{
    string Text;

    for (const auto& File : PerFile)
    {
        if (File->PerFrame.empty() || File->PerChange.empty())
            continue; // Show the file only if there is some DV content

        // File header and style
        Text += "WEBVTT\n"
            "Style:\n"
            "::cue{\n"
            "  line - height: 5.33vh;\n"
            "  font - size: 4.1vh;\n"
            "  font - family: monospace;\n"
            "  font - style: normal;\n"
            "  font - weight: normal;\n"
            "   background - color: black;\n"
            "  color: white;\n"
            "}\n"
            "##\n"
            "Kind: captions\n"
            "Language : en - US\n";

        // By Frame - For each line
        auto FrameNumber_Max = File->PerFrame.size() - 1;
        string TimeStamp_String;
        seconds_to_timestamp(TimeStamp_String, 0, 3, true);
        string TimeStamp2_String;
        for (const auto& Frame : File->PerFrame)
        {
            decltype(FrameNumber_Max) FrameNumber = &Frame - &*File->PerFrame.begin();

            //if (ShowFrame) // Actually always show frame
            {
                // Empty line
                Text += '\n';

                auto TimeStamp2 = (FrameNumber + 1) / File->FrameRate;
                seconds_to_timestamp(TimeStamp2_String, TimeStamp2, 3, true);

                Text += TimeStamp_String;
                Text += " --> ";
                Text += TimeStamp2_String;
                TimeStamp_String = move(TimeStamp2_String);
                Text += '\n';

                // TimeCode
                auto TimeCode_Seconds = (Frame->TimeCode >> 8) & 0x7FFFF; // Value
                if (TimeCode_Seconds != 0x7FFFF)
                {
                    auto TimeCode_DropFrame = Frame->TimeCode & 0x00000080 ? true : false;
                    auto TimeCode_Frames = Frame->TimeCode & 0x3F;
                    Text += "TC=";
                    timecode_to_string(Text, TimeCode_Seconds, TimeCode_DropFrame, TimeCode_Frames);
                }
                if ((Frame->TimeCode >> 31) & 0x1) // Repeat
                {
                    Text += "(R)";
                }
                if ((Frame->TimeCode >> 30) & 0x1) // Non consecutive
                {
                    Text += "(NC)";
                }

                // RecDate/RecTime
                string RecDateTime_String;
                auto RecDateTime_Years = (Frame->RecordedDateTime1 >> 17) & 0x7F;
                if (RecDateTime_Years != 0x7F)
                {
                    auto RecDateTime_Months = (Frame->RecordedDateTime2 >> 12) & 0x0F;
                    auto RecDateTime_Days = (Frame->RecordedDateTime2 >> 8) & 0x1F;
                    date_to_string(RecDateTime_String, RecDateTime_Years, RecDateTime_Months, RecDateTime_Days);
                }
                auto RecDateTime_Seconds = Frame->RecordedDateTime1 & 0x1FFFF;
                if (RecDateTime_Seconds != 0x1FFFF)
                {
                    if (!RecDateTime_String.empty())
                        RecDateTime_String += ' ';
                    auto RecDateTime_DropFrame = Frame->TimeCode & 0x00000080 ? true : false;
                    auto RecDateTime_Frames = Frame->RecordedDateTime2 & 0x7F;
                    timecode_to_string(RecDateTime_String, RecDateTime_Seconds, RecDateTime_DropFrame, RecDateTime_Frames);
                }
                if (!RecDateTime_String.empty())
                {
                    if (TimeCode_Seconds != 0x7FFFF)
                        Text += ' ';
                    Text += "RDT=" + RecDateTime_String;
                }
                if ((Frame->RecordedDateTime1 >> 31) & 0x1) // Repeat
                {
                    Text += "(R)";
                }
                if ((Frame->RecordedDateTime1 >> 30) & 0x1) // Non consecutive
                {
                    Text += "(NC)";
                }
                if (Frame->RecordedDateTime1 & (1 << 29)) // Start
                {
                    Text += "(Start)";
                }
                if (Frame->RecordedDateTime1 & (1 << 28)) // End
                {
                    Text += "(End)";
                }

                // Arb
                auto Arb = Frame->Arb;
                if (Arb & (1 << 4)) // Value
                {
                    if (TimeCode_Seconds != 0x7FFFF || !RecDateTime_String.empty())
                        Text += ' ';
                    auto Arb_Value = Arb & 0xF;
                    Text += string("ARB=") + uint4_to_hex4(Arb_Value);
                }
                if (Arb & (1 << 7)) // Repeat
                {
                    Text += ("(R)");
                }
                if (Arb & (1 << 6)) // Non consecutive
                {
                    Text += ("(NC)");
                }
                Text += '\n';

                // Errors
                if ((Frame->Video_STA_Errors && Frame->Video_STA_Errors_Count == DseqSta_Size) || (Frame->Audio_Data_Errors && Frame->Audio_Data_Errors_Count == Dseq_Size))
                {
                    // Compute
                    computed_errors ComputedErrors;
                    ComputedErrors.Compute(*Frame);

                    // Display
                    auto Size_Before = Text.size();
                    Xml_Sta_Elements(Text, ComputedErrors.Video_Sta_TotalPerSta, ComputedErrors.Video_Sta_EvenTotalPerSta);
                    Xml_Aud_Element(Text, ComputedErrors.Audio_Data_Total, ComputedErrors.Audio_Data_EvenTotal);
                    auto Size_After = Text.size();
                    if (Size_After > Size_Before + 51)
                    {
                        Text.resize(Size_Before + 48);
                        Text.append(3, '.');
                    }
                    if (Size_After == Size_Before)
                        Text += ' '; // Actually no error
                }
                else
                    Text += ' ';
                Text += '\n';

                // Animation
                auto FrameNumber_Anim_Current = FrameNumber;
                if (FrameNumber_Anim_Current < 25)
                {
                    Text.append(25 - FrameNumber_Anim_Current, ' ');
                    FrameNumber_Anim_Current = 0;
                }
                else
                    FrameNumber_Anim_Current -= 25;
                for (; FrameNumber_Anim_Current < FrameNumber; FrameNumber_Anim_Current++)
                {
                    auto& Frame_Anim = File->PerFrame[FrameNumber_Anim_Current];
                    if (Frame_Anim->Video_STA_Errors || Frame_Anim->Audio_Data_Errors)
                        Text += 'x';
                    else
                        Text += '-';
                }
                Text += '|';
                FrameNumber_Anim_Current++;
                auto FrameNumber_Anim_Max = FrameNumber_Anim_Current + 25;
                decltype(FrameNumber_Max) PaddingCount;
                if (FrameNumber_Anim_Max > FrameNumber_Max)
                {
                    PaddingCount = FrameNumber_Anim_Max - FrameNumber_Max;
                    FrameNumber_Anim_Max = FrameNumber_Max;
                }
                else
                    PaddingCount = 0;
                for (; FrameNumber_Anim_Current <= FrameNumber_Anim_Max; FrameNumber_Anim_Current++)
                {
                    auto& Frame_Anim = File->PerFrame[FrameNumber_Anim_Current];
                    if (Frame_Anim->Video_STA_Errors || Frame_Anim->Audio_Data_Errors)
                        Text += 'x';
                    else
                        Text += '-';
                }
                if (PaddingCount)
                    Text.append(PaddingCount, ' ');

                Text += '\n';

                // Write content to output
                if (auto ToReturn = WriteIfBig(Out, Text, Err, Writer_Name))
                    return ToReturn;
            }

            FrameNumber++;
        }
    }

    // Write content to output
    return Write(Out, Text, Err, Writer_Name);
}
