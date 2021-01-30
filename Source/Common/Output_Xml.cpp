/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/Output.h"
#include "Common/Output_Xml.h"
#include "Common/ProcessFile.h"
#include "ZenLib/Ztring.h"
#include <bitset>
#include <cstddef>
#include <map>
#include <queue>
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
return_value Output_Xml(ostream& Out, std::vector<file*>& PerFile, bitset<Option_Max> Options, ostream* Err)
{
    string Text;

    // XML header
    Text += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<dvrescue xmlns=\"https://mediaarea.net/dvrescue\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"https://mediaarea.net/dvrescue https://mediaarea.net/dvrescue/dvrescue.xsd\" version=\"1.1\">\n"
        "\t<creator>\n"
        "\t\t<program>dvrescue</program>\n"
        "\t\t<version>" Program_Version "</version>\n"
        "\t</creator>\n";

    for (const auto& File : PerFile)
    {
        // Media header
        auto FileName = File->MI.Get(Stream_General, 0, __T("CompleteName"));
        if (FileName.empty())
            continue; // Show the file only if it exists
        Text += "\t<media ref=\"";
        Text += Ztring(FileName).To_UTF8();;
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
        auto Format = File->MI.Get(Stream_General, 0, __T("Format"));
        if (!Format.empty())
        {
            Text += " format=\"";
            Text += Ztring(Format).To_UTF8();
            Text += '\"';
        }
        auto FileSize = File->MI.Get(Stream_General, 0, __T("FileSize"));
        if (!FileSize.empty())
        {
            Text += " size=\"";
            Text += Ztring(FileSize).To_UTF8();
            Text += '\"';
        }
        Text += ">\n";

        // By Frame - For each line
        auto FrameNumber_Max = File->PerFrame.size() - 1;
        auto PerChange_Next = File->PerChange.begin();
        auto ShowFrames = true;
        queue<size_t> Captions_Partial[2]; // 0 = Out, 1 = In
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
                if (!Options[Option_CaptionPresenceChange] && PerChange_Next != File->PerChange.end())
                {
                    Captions_Partial[0] = {};
                    Captions_Partial[1] = {};
                    bool HasChanges = false;
                    bool CaptionsOn = Change->Captions_Flags & 1;
                    Captions_Partial[CaptionsOn].push(Change->FrameNumber);
                    do
                    {
                        // We check if the caption presence change is the only change, and skip it if it is the case, while keeping info about in/out of caption change
                        constexpr auto Size_Before = offsetof(MediaInfo_Event_DvDif_Change_0, Captions_Flags) - sizeof(MediaInfo_Event_Generic);
                        constexpr auto Offset_After = offsetof(MediaInfo_Event_DvDif_Change_0, Captions_Flags) + sizeof(MediaInfo_Event_DvDif_Change_0::Captions_Flags);
                        const auto Size_After = Change->EventSize - Offset_After;
                        if ((Size_Before && memcmp((const char*)&Change->Captions_Flags - Size_Before, (const char*)&(*PerChange_Next)->Captions_Flags - Size_Before, Size_Before))
                            || (Size_After && memcmp((const char*)&Change->Captions_Flags + sizeof(MediaInfo_Event_DvDif_Change_0::Captions_Flags), (const char*)&(*PerChange_Next)->Captions_Flags + sizeof(MediaInfo_Event_DvDif_Change_0::Captions_Flags), Size_After))
                            || ((Change->Captions_Flags&(~1)) != ((*PerChange_Next)->Captions_Flags&(~1)))) // Any bit but bit 0
                                break;
                        HasChanges = true;
                        CaptionsOn = !CaptionsOn;
                        if (CaptionsOn || !Captions_Partial[1].empty())
                            Captions_Partial[CaptionsOn].push((*PerChange_Next)->FrameNumber);
                        PerChange_Next++;
                    }
                    while (PerChange_Next != File->PerChange.end());
                    if (!HasChanges)
                    {
                        Captions_Partial[0] = {};
                        Captions_Partial[1] = {};
                    }
                }
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
                if (!Captions_Partial[0].empty() || !Captions_Partial[1].empty())
                {
                    Text += " captions=\"p\"";
                }
                else if (Change->Captions_Flags & 0x1)
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

            if ((!Captions_Partial[0].empty() && FrameNumber == Captions_Partial[0].front())
                || (!Captions_Partial[1].empty() && FrameNumber == Captions_Partial[1].front()))
            {
                ShowFrame = true;
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
                if (Frame->StreamOffset != MediaInfo_int64u(-1))
                {

                    Text += " pos=\"";
                    Text += to_string(Frame->StreamOffset);
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
                bool Caption_On = !Captions_Partial[1].empty() && FrameNumber == Captions_Partial[1].front();
                bool Caption_Off = !Captions_Partial[0].empty() && FrameNumber == Captions_Partial[0].front();
                if (Caption_On)
                {
                    Text += " caption=\"on\"";
                }
                if (Caption_Off)
                {
                    Text += " caption=\"off\"";
                }
                if (Caption_On)
                {
                    Captions_Partial[1].pop();
                }
                if (Caption_Off)
                {
                    Captions_Partial[0].pop();
                }
                if (Frame->Captions_Errors & 1)
                {
                    Text += " caption-parity=\"mismatch\"";
                }

                // Coherency
                coherency_flags Coherency(Frame->Coherency_Flags);
                if (Coherency.no_pack())
                {
                    Text += " no_pack=\"1\"";
                }
                else
                {
                    if (Coherency.no_pack_vid())
                    {
                        Text += " no_pack_vid=\"1\"";
                    }
                    else
                    {
                        if (Coherency.no_sourceorcontrol_vid())
                        {
                            Text += " no_sourceorcontrol_vid=\"1\"";
                        }
                    }
                    if (Coherency.no_pack_aud())
                    {
                        Text += " no_pack_aud=\"1\"";
                    }
                    else
                    {
                        if (Coherency.no_sourceorcontrol_aud())
                        {
                            Text += " no_sourceorcontrol_aud=\"1\"";
                        }
                    }
                }
                if (Coherency.full_conceal())
                {
                    Text += " full_conceal=\"1\"";
                }
                else
                {
                    if (Coherency.full_conceal_vid())
                    {
                        Text += " full_conceal_vid=\"1\"";
                    }
                    if (Coherency.full_conceal_aud())
                    {
                        Text += " full_conceal_aud=\"1\"";
                    }
                }

                // Errors
                if (!Coherency.full_conceal() && ((!Coherency.full_conceal_vid() && Frame->Video_STA_Errors && Frame->Video_STA_Errors_Count == DseqSta_Size) || (!Coherency.full_conceal_aud() && Frame->Audio_Data_Errors && Frame->Audio_Data_Errors_Count == Dseq_Size)))
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
                            if (!Coherency.full_conceal_vid())
                                Sta_Elements(Text, 5, ComputedErrors.PerDseq.Video_Sta_TotalPerSta);
                            if (!Coherency.full_conceal_aud())
                                Aud_Element(Text, 5, ComputedErrors.PerDseq.Audio_Data_Total);
                            Dseq_End(Text, 4);
                        }
                    }

                    // Display
                    if (!Coherency.full_conceal_vid())
                        Sta_Elements(Text, 4, ComputedErrors.Video_Sta_TotalPerSta, ComputedErrors.Video_Sta_EvenTotalPerSta);
                    if (!Coherency.full_conceal_aud())
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
