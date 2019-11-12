/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a Unlicense license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/XmlOutput.h"
#include "Common/ProcessFile.h"
#include "ZenLib/Ztring.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

//***************************************************************************
// Sizes
//***************************************************************************

const auto Sta_Bits = 4;
const auto Dseq_Bits = 4;
const auto Sta_Size = 1 << Sta_Bits;
const auto Dseq_Size = 1 << Dseq_Bits;
const auto DseqSta_Size = Dseq_Size * Sta_Size;

//***************************************************************************
// Strings
//***************************************************************************

const size_t chroma_subsampling_size = 3;
const char* chroma_subsampling[chroma_subsampling_size] =
{
    "4:1:1",
    "4:2:0",
    "4:2:2",
};

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
string TimeCode2String(int Seconds, bool DropFrame, int Frames)
{
    string Value("00:00:00:00");
    Value[0] += Seconds / 36000; Seconds %= 36000;
    Value[1] += Seconds / 3600; Seconds %= 3600;
    Value[3] += Seconds / 600; Seconds %= 600;
    Value[4] += Seconds / 60; Seconds %= 60;
    Value[6] += Seconds / 10; Seconds %= 10;
    Value[7] += Seconds;
    if (Frames < 100)
    {
        if (DropFrame)
            Value[8] = ';';
        Value[9] += Frames / 10;
        Value[10] += Frames % 10;
    }
    else
        Value.resize(8);

    return Value;
}

//---------------------------------------------------------------------------
string to_timestamp(double Seconds_Float)
{
    if (Seconds_Float >= 360000)
        return string(); // Not supported
    string Value("00:00:00.000");
    auto Seconds = int(Seconds_Float);
    Seconds_Float -= Seconds;
    Seconds_Float *= 1000;
    auto MilliSeconds = int(Seconds_Float);
    Seconds_Float -= MilliSeconds;
    if (Seconds_Float >= 0.5)
        MilliSeconds++;
    Value[0] += Seconds / 36000; Seconds %= 36000;
    Value[1] += Seconds / 3600; Seconds %= 3600;
    Value[3] += Seconds / 600; Seconds %= 600;
    Value[4] += Seconds / 60; Seconds %= 60;
    Value[6] += Seconds / 10; Seconds %= 10;
    Value[7] += Seconds;
    Value[9] += MilliSeconds / 100; MilliSeconds %= 100;
    Value[10] += MilliSeconds / 10; MilliSeconds %= 10;
    Value[11] += MilliSeconds;

    return Value;
}

//---------------------------------------------------------------------------
string Date2String(int Years, int Months, int Days)
{
    string Value("2000-00-00");
    if (Years >= 70) // Arbitrary decided
    {
        Value[0] = '1';
        Value[1] = '9';
    }
    Value[2] += Years / 10;
    Value[3] += Years % 10;
    Value[5] += Months / 10;
    Value[6] += Months % 10;
    Value[8] += Days / 10;
    Value[9] += Days % 10;

    return Value;
}

//---------------------------------------------------------------------------
char to_hex4(int Value)
{
    if (Value >= 16)
        return 'X';
    if (Value >= 10)
        return 'A' - 10 + Value;
    return '0' + Value;
}

void Xml_Dseq_Begin(string& Text, size_t o, int Dseq)
{
    Text.append(o, '\t');
    Text += "<dseq n=\"";
    Text += to_string(Dseq);
    Text += "\">\n";
}

void Xml_Dseq_End(string& Text, size_t o)
{
    Text.append(o, '\t');
    Text += "</dseq>\n";
}

void Xml_Sta_Element(string& Text, size_t o, int Sta, size_t n, size_t n_even = size_t(-1))
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

void Xml_Sta_Elements(string& Text, size_t o, const size_t* const Stas, const size_t* const Stas_even = nullptr)
{
    for (auto Sta = 0; Sta < Sta_Size; Sta++)
    {
        auto n = Stas[Sta];
        auto n_even = Stas_even == nullptr ? size_t(-1) : Stas_even[Sta];
        Xml_Sta_Element(Text, o, Sta, n, n_even);
    }
}

void Xml_Aud_Element(string& Text, size_t o, size_t n, size_t n_even = size_t(-1))
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
string OutputXml(std::vector<file*>& PerFile)
{
    string Text;

    // XML header
    Text += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<dvrescue xmlns=\"https://mediaarea.net/dvrescue\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"https://mediaarea.net/dvrescue https://raw.githubusercontent.com/mipops/dvrescue/schema-updates/tools/dvrescue.xsd\" version=\"1.0\">\n"
        "\t<creator>\n"
        "\t\t<program>dvrescue</program>\n"
        "\t\t<version>" Program_Version "</version>\n"
        "\t</creator>\n";

    for (const auto& File : PerFile)
    {
        if (File->PerFrame.empty() || File->PerChange.empty())
            continue; // Show the file only if there is some DV content

        // Media header
        Text += "\t<media ref=\"" + Ztring(File->MI.Get(Stream_General, 0, __T("CompleteName"))).To_UTF8() + "\">\n";

        // By Frame - For each line
        auto FrameNumber_Max = File->PerFrame.size() - 1;
        auto PerChange_Next = File->PerChange.begin();
        auto ShowFrames = true;
        for (const auto& Frame : File->PerFrame)
        {
            decltype(FrameNumber_Max) FrameNumber = &Frame - &*File->PerFrame.begin();
            auto ShowFrame = ShowFrames || Frame->Video_STA_Errors || Frame->Audio_Data_Errors || FrameNumber == FrameNumber_Max;

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
                    Text += to_timestamp(TimeStamp_Begin);
                    Text += '\"';
                }
                {
                    auto TimeStamp_End = (PerChange_Next != File->PerChange.end() ? (*PerChange_Next)->FrameNumber : (FrameNumber_Max + 1)) / File->FrameRate;
                    Text += " end_pts=\"";
                    Text += to_timestamp(TimeStamp_End);
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
                Text += ">\n";
            }

            if (!ShowFrame)
            {
                if (PerChange_Next != File->PerChange.end() && (*PerChange_Next)->FrameNumber == FrameNumber + 1)
                {
                    ShowFrame = true;
                    ShowFrames = true; // For next frame
                }
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
                    Text += to_timestamp(TimeStamp);
                    Text += '\"';
                }

                // TimeCode
                auto TimeCode_Seconds = (Frame->TimeCode >> 8) & 0x7FFFF; // Value
                if (TimeCode_Seconds != 0x7FFFF)
                {
                    auto TimeCode_DropFrame = Frame->TimeCode & 0x00000080 ? true : false;
                    auto TimeCode_Frames = Frame->TimeCode & 0x3F;
                    Text += " tc=\"" + TimeCode2String(TimeCode_Seconds, TimeCode_DropFrame, TimeCode_Frames) + "\"";
                }
                if ((Frame->TimeCode >> 31) & 0x1) // Repeat
                {
                    Text += " tc_r=\"1\"";
                }
                if ((Frame->TimeCode >> 30) & 0x1) // Non consecutive
                {
                    Text += " tc_nc=\"1\"";
                }

                // RecDate/RecTime
                string RecDateTime_String;
                auto RecDateTime_Years = (Frame->RecordedDateTime1 >> 17) & 0x7F;
                if (RecDateTime_Years != 0x7F)
                {
                    auto RecDateTime_Months = (Frame->RecordedDateTime2 >> 12) & 0x0F;
                    auto RecDateTime_Days = (Frame->RecordedDateTime2 >> 8) & 0x1F;
                    RecDateTime_String = Date2String(RecDateTime_Years, RecDateTime_Months, RecDateTime_Days);
                }
                auto RecDateTime_Seconds = Frame->RecordedDateTime1 & 0x1FFFF;
                if (RecDateTime_Seconds != 0x1FFFF)
                {
                    if (!RecDateTime_String.empty())
                        RecDateTime_String += ' ';
                    auto RecDateTime_DropFrame = Frame->TimeCode & 0x00000080 ? true : false;
                    auto RecDateTime_Frames = Frame->RecordedDateTime2 & 0x7F;
                    RecDateTime_String += TimeCode2String(RecDateTime_Seconds, RecDateTime_DropFrame, RecDateTime_Frames);
                }
                if (!RecDateTime_String.empty())
                    Text += " rdt=\"" + RecDateTime_String + "\"";
                if ((Frame->RecordedDateTime1 >> 31) & 0x1) // Repeat
                {
                    Text += " rdt_r=\"1\"";
                }
                if ((Frame->RecordedDateTime1 >> 30) & 0x1) // Non consecutive
                {
                    Text += " rdt_nc=\"1\"";
                }
                if (Frame->RecordedDateTime1&(1 << 29)) // Start
                {
                    Text += " rec_start=\"1\"";
                }
                if (Frame->RecordedDateTime1&(1 << 28)) // End
                {
                    Text += " rec_end=\"1\"";
                }

                // Arb
                auto Arb = Frame->Arb;
                if (Arb & (1 << 4)) // Value
                {
                    auto Arb_Value = Arb & 0xF;
                    char Arb_Char;
                    if (Arb_Value < 10)
                        Arb_Char = '0' + Arb_Value;
                    else 
                        Arb_Char = 'A' + Arb_Value - 10;
                    Text += string(" arb=\"") + Arb_Char + "\"";
                }
                if (Arb & (1 << 7)) // Repeat
                {
                    Text += (" arb_r=\"1\"");
                }
                if (Arb & (1 << 6)) // Non consecutive
                {
                    Text += (" arb_nc=\"1\"");
                }

                // Errors
                if (Frame->Video_STA_Errors || Frame->Audio_Data_Errors)
                {
                    Text += ">\n";

                    // Split
                    if (Frame->Video_STA_Errors_Count == DseqSta_Size || Frame->Audio_Data_Errors_Count == Dseq_Size)
                    {
                        // Compute
                        size_t Video_Sta_TotalPerSta[Sta_Size];
                        memset(Video_Sta_TotalPerSta, 0, Sta_Size * sizeof(size_t));
                        size_t Video_Sta_EvenTotalPerSta[Sta_Size];
                        memset(Video_Sta_EvenTotalPerSta, 0, Sta_Size * sizeof(size_t));
                        size_t Audio_Data_Total = 0;
                        size_t Audio_Data_EvenTotal = 0;
                        for (auto Dseq = 0; Dseq < Dseq_Size; Dseq++)
                        {
                            // Compute
                            size_t Video_Sta_TotalPerDseqPerSta[Sta_Size];
                            memset(Video_Sta_TotalPerDseqPerSta, 0, Sta_Size * sizeof(size_t));
                            size_t Video_Sta_EvenTotalPerDseqPerSta[Sta_Size];
                            memset(Video_Sta_EvenTotalPerDseqPerSta, 0, Sta_Size * sizeof(size_t));
                            size_t Audio_Data_TotalPerDseq[Dseq_Size];
                            memset(Audio_Data_TotalPerDseq, 0, Dseq_Size * sizeof(size_t));
                            size_t Audio_Data_EvenTotalPerDseq[Dseq_Size];
                            memset(Audio_Data_EvenTotalPerDseq, 0, Dseq_Size * sizeof(size_t));
                            bool HasErrors = false;
                            for (auto Sta = 0; Sta < Sta_Size; Sta++)
                            {
                                if (Frame->Video_STA_Errors)
                                {
                                    auto DseqSta = (Dseq << Sta_Bits) | Sta;
                                    const auto n = Frame->Video_STA_Errors[DseqSta];
                                    if (n)
                                    {
                                        if (!HasErrors)
                                            HasErrors = true;
                                        Video_Sta_TotalPerDseqPerSta[Sta] += n;
                                        Video_Sta_TotalPerSta[Sta] += n;
                                        if (!(Dseq % 2))
                                        {
                                            Video_Sta_EvenTotalPerDseqPerSta[Sta] += n;
                                            Video_Sta_EvenTotalPerSta[Sta] += n;
                                        }
                                    }
                                }
                            }
                            if (Frame->Audio_Data_Errors)
                            {
                                const auto n = Frame->Audio_Data_Errors[Dseq];
                                if (n)
                                {
                                    if (!HasErrors)
                                        HasErrors = true;
                                    Audio_Data_TotalPerDseq[Dseq] += n;
                                    Audio_Data_Total += n;
                                    if (!(Dseq % 2))
                                    {
                                        Audio_Data_EvenTotalPerDseq[Dseq] += n;
                                        Audio_Data_EvenTotal += n;
                                    }
                                }
                            }

                            // Display
                            if (HasErrors)
                            {
                                Xml_Dseq_Begin(Text, 4, Dseq);
                                Xml_Sta_Elements(Text, 5, Video_Sta_TotalPerDseqPerSta);
                                Xml_Aud_Element(Text, 5, Audio_Data_TotalPerDseq[Dseq]);
                                Xml_Dseq_End(Text, 4);
                            }
                        }

                        // Display
                        Xml_Sta_Elements(Text, 4, Video_Sta_TotalPerSta, Video_Sta_EvenTotalPerSta);
                        Xml_Aud_Element(Text, 4, Audio_Data_Total, Audio_Data_EvenTotal);
                    }

                    Text += "\t\t\t</frame>\n";
                }
                else
                    Text += "/>\n";
            }
        }

        // Media footer
        Text += "\t\t</frames>\n";
        Text += "\t</media>\n";
    }

    // XML footer
    Text += "</dvrescue>\n";

    return Text;
}
