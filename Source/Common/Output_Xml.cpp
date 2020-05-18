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
using namespace ZenLib;
//---------------------------------------------------------------------------

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
static const char* const Writer_Name = "XML";

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
static void Dseq_Begin(string& Text, size_t o, int Dseq)
{
    Text.append(o, '\t');
    Text += "<dseq n=\"";
    Text += to_string(Dseq);
    Text += "\">\n";
}

//---------------------------------------------------------------------------
static void Dseq_End(string& Text, size_t o)
{
    Text.append(o, '\t');
    Text += "</dseq>\n";
}

//---------------------------------------------------------------------------
static void Sta_Element(string& Text, size_t o, int Sta, size_t n, size_t n_even = size_t(-1))
{
    if (!n)
        return;

    Text.append(o, '\t');
    Text += "<sta t=\"";
    Text += to_string(Sta);
    Text += "\" n=\"";
    Text += to_string(n);
    Text += "\"";
    if (n_even != size_t(-1))
    {
        Text += " n_even=\"";
        Text += to_string(n_even);
        Text += '\"';
    }
    Text += "/>\n";
}

//---------------------------------------------------------------------------
static void Sta_Elements(string& Text, size_t o, const size_t* const Stas, const size_t* const Stas_even = nullptr)
{
    for (auto Sta = 0; Sta < Sta_Size; Sta++)
    {
        auto n = Stas[Sta];
        auto n_even = Stas_even == nullptr ? size_t(-1) : Stas_even[Sta];
        Sta_Element(Text, o, Sta, n, n_even);
    }
}

//---------------------------------------------------------------------------
static void Aud_Element(string& Text, size_t o, size_t n, size_t n_even = size_t(-1))
{
    if (!n)
        return;

    Text.append(o, '\t');
    Text += "<aud n=\"";
    Text += to_string(n);
    Text += "\"";
    if (n_even != size_t(-1))
    {
        Text += " n_even=\"";
        Text += to_string(n_even);
        Text += '\"';
    }
    Text += "/>\n";
}

//***************************************************************************
// Output
//***************************************************************************

//---------------------------------------------------------------------------
return_value Output_Xml(ostream& Out, std::vector<file*>& PerFile, ostream* Err)
{
    string Text;

    // XML header
    Text += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<dvrescue xmlns=\"https://mediaarea.net/dvrescue\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"https://mediaarea.net/dvrescue https://mediaarea.net/dvrescue/dvrescue.xsd\" version=\"1.0\">\n"
        "\t<creator>\n"
        "\t\t<program>dvrescue</program>\n"
        "\t\t<version>" Program_Version "</version>\n"
        "\t</creator>\n";

    for (const auto& File : PerFile)
    {
        // Media header
        auto FileName = Ztring(File->MI.Get(Stream_General, 0, __T("CompleteName"))).To_UTF8();
        if (FileName.empty())
            continue; // Show the file only if it exists
        Text += "\t<media ref=\"";
        Text += FileName;
        Text += '\"';
        if (File->PerFrame.empty() || File->PerChange.empty())
        {
            if (File->MI.Get(Stream_Video, 0, __T("Format")) != __T("DV"))
                Text += " error=\"not DV\"";
            else
                Text += " error=\"no frame received\"";
            Text += "/>\n";
            continue; // Show the file only if there is some DV content
        }
        Text += ">\n";

        // By Frame - For each line
        auto FrameNumber_Max = File->PerFrame.size() - 1;
        auto PerChange_Next = File->PerChange.begin();
        auto ShowFrames = true;
        for (const auto& Frame : File->PerFrame)
        {
            decltype(FrameNumber_Max) FrameNumber = &Frame - &*File->PerFrame.begin();
            auto ShowFrame = ShowFrames || FrameNumber == FrameNumber_Max || Frame_HasErrors(Frame);

            if (ShowFrames)
            {
                ShowFrames = false;

                if (FrameNumber)
                    Text += "\t\t</frames>\n";

                const auto Change = *PerChange_Next;
                PerChange_Next++;
                Text += "\t\t<frames";
                {
                    auto FrameCount = (PerChange_Next != File->PerChange.end() ? (*PerChange_Next)->FrameNumber : (FrameNumber_Max + 1)) - FrameNumber;
                    Text += " count=\"";
                    Text += to_string(FrameCount);
                    Text += '\"';
                }
                {
                    auto TimeStamp_Begin = FrameNumber / File->FrameRate;
                    Text += " pts=\"";
                    seconds_to_timestamp(Text, TimeStamp_Begin, 6);
                    Text += '\"';
                }
                {
                    auto TimeStamp_End = (PerChange_Next != File->PerChange.end() ? (*PerChange_Next)->FrameNumber : (FrameNumber_Max + 1)) / File->FrameRate;
                    Text += " end_pts=\"";
                    seconds_to_timestamp(Text, TimeStamp_End, 6);
                    Text += '\"';
                }
                if (Change->Width && Change->Height)
                {
                    Text += " size=\"";
                    Text += to_string(Change->Width);
                    Text += 'x';
                    Text += to_string(Change->Height);
                    Text += '\"';
                }
                if (Change->VideoRate_N)
                {
                    Text += " video_rate=\"";
                    Text += to_string(Change->VideoRate_N);
                    if (Change->VideoRate_D && Change->VideoRate_D != 1)
                    {
                        Text += '/';
                        Text += to_string(Change->VideoRate_D);
                    }
                    Text += '\"';
                }
                if (Change->VideoChromaSubsampling <= chroma_subsampling_size)
                {
                    Text += " chroma_subsampling=\"";
                    Text += chroma_subsampling[Change->VideoChromaSubsampling];
                    Text += '\"';
                }
                if (Change->VideoRatio_N)
                {
                    Text += " aspect_ratio=\"";
                    Text += to_string(Change->VideoRatio_N);
                    if (Change->VideoRatio_D && Change->VideoRatio_D != 1)
                    {
                        Text += '/';
                        Text += to_string(Change->VideoRatio_D);
                    }
                    Text += '\"';
                }
                if (Change->AudioRate_N)
                {
                    Text += " audio_rate=\"";
                    Text += to_string(Change->AudioRate_N);
                    if (Change->AudioRate_D && Change->AudioRate_D != 1)
                    {
                        Text += '/';
                        Text += to_string(Change->AudioRate_D);
                    }
                    Text += '\"';
                }
                if (Change->AudioChannels)
                {
                    Text += " channels=\"";
                    Text += to_string(Change->AudioChannels);
                    Text += '\"';
                }
                if (Change->Captions_Flags & 0x1)
                {
                    Text += " captions=\"y\"";
                }
                Text += ">\n";
            }

            if (PerChange_Next != File->PerChange.end() && (*PerChange_Next)->FrameNumber == FrameNumber + 1)
            {
                ShowFrame = true;
                ShowFrames = true; // For next frame
            }

            if (ShowFrame)
            {
                auto TimeStamp = FrameNumber / File->FrameRate;

                Text += "\t\t\t<frame";
                {
                    Text += " n=\"";
                    Text += to_string(FrameNumber);
                    Text += '\"';
                }
                {
                    Text += " pts=\"";
                    seconds_to_timestamp(Text, TimeStamp, 6);
                    Text += '\"';
                }

                // TimeCode
                auto TimeCode = timecode(Frame->TimeCode);
                if (TimeCode.HasValue())
                {
                    Text += " tc=\"";
                    timecode_to_string(Text, TimeCode.TimeInSeconds(), TimeCode.DropFrame(), TimeCode.Frames());
                    Text += '\"';
                }
                if (TimeCode.Repeat())
                {
                    Text += " tc_r=\"1\"";
                }
                if (TimeCode.NonConsecutive())
                {
                    Text += " tc_nc=\"1\"";
                }

                // RecDate/RecTime
                rec_date_time RecDateTime(Frame->RecordedDateTime1, Frame->RecordedDateTime2);
                string RecDateTime_String;
                if (RecDateTime.HasDate())
                {
                    date_to_string(RecDateTime_String, RecDateTime.Years(), RecDateTime.Months(), RecDateTime.Days());
                }
                if (RecDateTime.HasTime())
                {
                    if (!RecDateTime_String.empty())
                        RecDateTime_String += ' ';
                    timecode_to_string(RecDateTime_String, RecDateTime.TimeInSeconds(), TimeCode.DropFrame(), RecDateTime.Frames());
                }
                if (!RecDateTime_String.empty())
                    Text += " rdt=\"" + RecDateTime_String + "\"";
                if (RecDateTime.Repeat())
                {
                    Text += " rdt_r=\"1\"";
                }
                if (RecDateTime.NonConsecutive())
                {
                    Text += " rdt_nc=\"1\"";
                }
                if (RecDateTime.Start())
                {
                    Text += " rec_start=\"1\"";
                }
                if (RecDateTime.End())
                {
                    Text += " rec_end=\"1\"";
                }

                // Arb
                auto Arb = frame_arb(Frame->Arb);
                if (Arb.HasValue())
                {
                    Text += string(" arb=\"") + uint4_to_hex4(Arb.Value()) + "\"";
                }
                if (Arb.Repeat())
                {
                    Text += " arb_r=\"1\"";
                }
                if (Arb.NonConsecutive())
                {
                    Text += " arb_nc=\"1\"";
                }

                // Captions
                if (Frame->Captions_Errors & 1)
                {
                    Text += " caption-parity=\"mismatch\"";
                }

                // Errors
                if ((Frame->Video_STA_Errors && Frame->Video_STA_Errors_Count == DseqSta_Size) || (Frame->Audio_Data_Errors && Frame->Audio_Data_Errors_Count == Dseq_Size))
                {
                    Text += ">\n";

                    // Compute
                    computed_errors ComputedErrors;

                    auto Size_Before = Text.size();
                    for (int Dseq = 0; Dseq < Dseq_Size; Dseq++)
                    {
                        if (ComputedErrors.Compute(*Frame, Dseq))
                        {
                            // Display
                            Dseq_Begin(Text, 4, Dseq);
                            Sta_Elements(Text, 5, ComputedErrors.PerDseq.Video_Sta_TotalPerSta);
                            Aud_Element(Text, 5, ComputedErrors.PerDseq.Audio_Data_Total);
                            Dseq_End(Text, 4);
                        }
                    }

                    // Display
                    Sta_Elements(Text, 4, ComputedErrors.Video_Sta_TotalPerSta, ComputedErrors.Video_Sta_EvenTotalPerSta);
                    Aud_Element(Text, 4, ComputedErrors.Audio_Data_Total, ComputedErrors.Audio_Data_EvenTotal);

                    auto Size_After = Text.size();
                    if (Size_After == Size_Before)
                        Text.insert(Text.size() - 2, 1, '/'); // Actually no error, self close the frame
                    else
                        Text += "\t\t\t</frame>\n";
                }
                else
                    Text += "/>\n";

                // Write content to output
                if (auto ToReturn = WriteIfBig(Out, Text, Err, Writer_Name))
                    return ToReturn;
            }
        }

        // Media footer
        Text += "\t\t</frames>\n";
        Text += "\t</media>\n";
    }

    // XML footer
    Text += "</dvrescue>\n";

    // Write content to output
    return Write(Out, Text, Err, Writer_Name);
}
