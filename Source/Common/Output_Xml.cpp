/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/Output.h"
#include "Common/Output_Xml.h"
#include "Common/ProcessFile.h"
#include "Common/ProcessFileWrapper.h"
#include "ThirdParty/TimeCode/TimeCode.h"
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
extern vector<string> Merge_InputFileNames;
extern vector<string> Merge_OutputFileNames;
extern uint64_t Merge_Out_Size;
extern uint64_t Timeout;
#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
extern uint8_t DeckLinkVideoSource;
extern uint8_t DeckLinkAudioSource;
extern uint8_t DeckLinkTimecodeFormat;
#endif

//---------------------------------------------------------------------------
static const char* const Writer_Name = "XML";

//***************************************************************************
// Formating helpers
//***************************************************************************

//---------------------------------------------------------------------------
static string to_hexstring(uint16_t value)
{
    const char* hex = "0123456789ABCDEF";
    string ToReturn("0x");
    ToReturn += hex[value >> 12 & 0xF];
    ToReturn += hex[value >>  8 & 0xF];
    ToReturn += hex[value >>  4 & 0xF];
    ToReturn += hex[value       & 0xF];
    return ToReturn;
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
enum format {
    Format_Unknown,
    Format_DV,
    Format_DAT,
};

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
static void Aud_Element(string& Text, size_t o, size_t n, vector<uint16_t> Audio_Errors_Values, size_t n_even = size_t(-1))
{
    if (!n)
        return;

    Text.append(o, '\t');
    Text += "<aud n=\"";
    Text += to_string(n);
    Text += "\"";
    if (!Audio_Errors_Values.empty())
    {
        Text += " t=\"2\" v=\"";
        for (size_t i = 0; i < Audio_Errors_Values.size(); i++)
        {
            if (i)
                Text += ' ';
            Text += to_hexstring(Audio_Errors_Values[i]);
        }
        Text += '\"';
    }
    if (n_even != size_t(-1))
    {
        Text += " n_even=\"";
        Text += to_string(n_even);
        Text += '\"';
    }
    Text += "/>\n";
}

#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
//---------------------------------------------------------------------------
string decklink_videosource_to_string(uint8_t value)
{
    string ToReturn;
    switch (value)
    {
    case (uint8_t)Decklink_Video_Source_SDI: ToReturn = "sdi"; break;
    case (uint8_t)Decklink_Video_Source_HDMI: ToReturn = "hdmi"; break;
    case (uint8_t)Decklink_Video_Source_Optical: ToReturn = "optical"; break;
    case (uint8_t)Decklink_Video_Source_Component: ToReturn = "component"; break;
    case (uint8_t)Decklink_Video_Source_Composite: ToReturn = "composite"; break;
    case (uint8_t)Decklink_Video_Source_SVideo: ToReturn = "s_video"; break;
    default:;
    }
    return ToReturn;
}

//---------------------------------------------------------------------------
string decklink_audiosource_to_string(uint8_t value)
{
    string ToReturn;
    switch (value)
    {
    case (uint8_t)Decklink_Audio_Source_Embedded: ToReturn = "embedded"; break;
    case (uint8_t)Decklink_Audio_Source_AESEBU: ToReturn = "aes_ebu"; break;
    case (uint8_t)Decklink_Audio_Source_Analog: ToReturn = "analog"; break;
    case (uint8_t)Decklink_Audio_Source_AnalogXLR: ToReturn = "analog_xlr"; break;
    case (uint8_t)Decklink_Audio_Source_AnalogRCA: ToReturn = "analog_rca"; break;
    case (uint8_t)Decklink_Audio_Source_Microphone: ToReturn = "microphone"; break;
    default:;
    }
    return ToReturn;
}

//---------------------------------------------------------------------------
string decklink_pixelformat_to_string(uint8_t value)
{
    string ToReturn;
    switch (value)
    {
    case (uint8_t)Decklink_Pixel_Format_8BitYUV: ToReturn = "uyvy"; break;
    case (uint8_t)Decklink_Pixel_Format_10BitYUV: ToReturn = "v210"; break;
    case (uint8_t)Decklink_Pixel_Format_8BitARGB: ToReturn = "argb"; break;
    case (uint8_t)Decklink_Pixel_Format_8BitBGRA: ToReturn = "bgra"; break;
    case (uint8_t)Decklink_Pixel_Format_10BitRGB: ToReturn = "r210"; break;
    default:;
    }
    return ToReturn;
}

//---------------------------------------------------------------------------
string decklink_timecodeformat_to_string(uint8_t value)
{
    string ToReturn;
    switch (value)
    {
    case (uint8_t)Decklink_Timecode_Format_RP188_VITC: ToReturn = "rp188_vitc"; break;
    case (uint8_t)Decklink_Timecode_Format_RP188_VITC2: ToReturn = "rp188_vitc2"; break;
    case (uint8_t)Decklink_Timecode_Format_RP188_LTC: ToReturn = "rp188_lpc"; break;
    case (uint8_t)Decklink_Timecode_Format_RP188_HFR: ToReturn = "rp188_hfr"; break;
    case (uint8_t)Decklink_Timecode_Format_RP188_ANY: ToReturn = "rp188_any"; break;
    case (uint8_t)Decklink_Timecode_Format_VITC: ToReturn = "vitc"; break;
    case (uint8_t)Decklink_Timecode_Format_VITC2: ToReturn = "vitc2"; break;
    case (uint8_t)Decklink_Timecode_Format_Serial: ToReturn = "serial"; break;
    default:;
    }
    return ToReturn;
}
#endif

//***************************************************************************
// Output
//***************************************************************************

//---------------------------------------------------------------------------
return_value Output_Xml(ostream& Out, std::vector<file*>& PerFile, bitset<Option_Max> Options, ostream* Err)
{
    string Text;

    // XML header
    Text += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<dvrescue xmlns=\"https://mediaarea.net/dvrescue\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"https://mediaarea.net/dvrescue https://mediaarea.net/dvrescue/dvrescue.xsd\" version=\"1.2.1\">\n"
        "\t<creator>\n"
        "\t\t<program>dvrescue</program>\n"
        "\t\t<version>" Program_Version "</version>\n"
        "\t\t<library version=\"" + MediaInfo_Version() + "\">MediaInfoLib</library>\n"
        "\t</creator>\n";

    for (const auto& File : PerFile)
    {
        // Media header
        if (File->CaptureMode == Capture_Mode_DV && !File->MI.Count_Get(Stream_General))
            continue; // Show the file only if it exists
        Text += "\t<media";
        auto FileName = File->CaptureMode == Capture_Mode_DV ? File->MI.Get(Stream_General, 0, __T("CompleteName")) : __T("");
        if (FileName.empty() && !Merge_OutputFileNames.empty())
            FileName = Ztring().From_Local(Merge_OutputFileNames[0]);
        if (!FileName.empty())
        {
            Text += " ref=\"";
            Text += Ztring(FileName).To_UTF8();
            Text += '\"';
        }
        if (!Merge_OutputFileNames.empty() && Merge_InputFileNames.size() == 1)
        {
            Text += " fromCapture=\"";
            Text += Merge_InputFileNames[0];
            Text += '\"';
        }
        auto Format_G = File->MI.Get(Stream_General, 0, __T("Format"));
        auto Format_V = File->MI.Get(Stream_Video, 0, __T("Format"));
        format Format;
        if (Format_G == __T("DAT"))
            Format = Format_DAT;
        else if (Format_V == __T("DV"))
            Format = Format_DV;
        else
            Format = Format_Unknown;
        if (File->CaptureMode == Capture_Mode_DV)
        {
            if (Format == Format_Unknown || File->PerFrame.empty() || File->PerChange.empty())
            {
                if (Format == Format_Unknown)
                    Text += " error=\"not supported\"";
                else
                    Text += " error=\"no frame received\"";
                Text += "/>\n";
                continue; // Show the file only if there is some DV content
            }
            if (!Format_G.empty())
            {
                Text += " format=\"";
                Text += Ztring(Format_G).To_UTF8();
                Text += '\"';
            }
            auto FileSize = File->MI.Get(Stream_General, 0, __T("FileSize"));
            if (FileSize.empty() && Merge_Out_Size != -1)
                FileSize = Ztring::ToZtring(Merge_Out_Size);
            if (!FileSize.empty())
            {
                Text += " size=\"";
                Text += Ztring(FileSize).To_UTF8();
                Text += '\"';
            }
        }
        #if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
        else if (File->CaptureMode == Capture_Mode_DeckLink)
        {
            if (!File->Capture || !File->Wrapper)
            {
                Text += " error=\"decklink initialization failed\"/>\n";
                continue;
            }
            else if (((FileWrapper*)File->Wrapper)->FramesInfo.frames.empty())
            {
                Text += " error=\"no frame received\"/>\n";
                continue; // Show the file only if there is some content
            }

            if (!Merge_Out.empty())
            {
                MediaInfo tmpMI;
                if (tmpMI.Open(Ztring().From_Local(Merge_OutputFileNames[0])))
                {
                    auto Format = tmpMI.Get(Stream_General, 0, __T("Format"));
                    if (!Format.empty())
                    {
                        Text += " format=\"";
                        Text += Ztring(Format).To_UTF8();
                        Text += '\"';
                    }

                    auto FileSize = tmpMI.Get(Stream_General, 0, __T("FileSize"));
                    if (!FileSize.empty())
                    {
                        Text += " size=\"";
                        Text += Ztring(FileSize).To_UTF8();
                        Text += '\"';
                    }
                }
            }
        }
        #endif
        Text += ">\n";

        if (File->TimeOutReached)
        {
            Text += "\t\t<stop method='timeout' extra='";
            Text += Ztring().From_Number(Timeout).To_UTF8();
            Text += "'/>\n";
        }
        else if (File->TerminateRequested)
            Text += "\t\t<stop method='user' extra='SIGINT'/>\n";

        #if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
        if (File->CaptureMode == Capture_Mode_DeckLink)
        {
            Text += "\t\t<source";
            auto Video = decklink_videosource_to_string(DeckLinkVideoSource);
            if (!Video.empty())
            {
                Text += " video=\"";
                Text += Video;
                Text += '\"';
            }

            auto Audio = decklink_audiosource_to_string(DeckLinkAudioSource);
            if (!Audio.empty())
            {
                Text += " audio=\"";
                Text += Audio;
                Text += '\"';
            }

            auto TimeCode = decklink_timecodeformat_to_string(DeckLinkTimecodeFormat);
            if (!TimeCode.empty())
            {
                Text += " timecode=\"";
                Text += TimeCode;
                Text += '\"';
            }
            Text += "/>\n";

            //Frames
            Text += "\t\t<frames";
            {
                Text += " count=\"";
                Text += to_string(File->Wrapper->FramesInfo.frames.size());
                Text += '\"';
            }
            if (!File->Wrapper->FramesInfo.frames.empty())
            {
                    Text += " pts=\"";
                    seconds_to_timestamp(Text, File->Wrapper->FramesInfo.frames[0].pts / 1000000000.0, 6, true);
                    Text += '\"';

                    Text += " end_pts=\"";
                    seconds_to_timestamp(Text, (File->Wrapper->FramesInfo.frames[File->Wrapper->FramesInfo.frames.size()-1].pts / 1000000000.0) + File->Wrapper->FramesInfo.frames[File->Wrapper->FramesInfo.frames.size()-1].dur, 6, true);
                    Text += '\"';
            }
            if (File->Wrapper->FramesInfo.video_width && File->Wrapper->FramesInfo.video_height)
            {
                Text += " size=\"";
                Text += to_string(File->Wrapper->FramesInfo.video_width);
                Text += 'x';
                Text += to_string(File->Wrapper->FramesInfo.video_height);
                Text += '\"';
            }

            if (File->Wrapper->FramesInfo.pixel_format)
            {
                Text += " pixel_format=\"";
                Text += decklink_pixelformat_to_string(File->Wrapper->FramesInfo.pixel_format);
                Text += '\"';
            }

            if (File->Wrapper->FramesInfo.video_rate_num)
            {
                Text += " video_rate=\"";
                Text += to_string(File->Wrapper->FramesInfo.video_rate_num);
                if (File->Wrapper->FramesInfo.video_rate_den && File->Wrapper->FramesInfo.video_rate_den != 1)
                {
                    Text += '/';
                    Text += to_string(File->Wrapper->FramesInfo.video_rate_den);
                }
                Text += '\"';
            }
            if (File->Wrapper->FramesInfo.audio_rate)
            {
                Text += " audio_rate=\"";
                Text += to_string(File->Wrapper->FramesInfo.audio_rate);
                Text += '\"';
            }
            if (File->Wrapper->FramesInfo.audio_channels)
            {
                Text += " channels=\"";
                Text += to_string(File->Wrapper->FramesInfo.audio_channels);
                Text += '\"';
            }
            Text += ">\n";

            // frame
            for (size_t Pos=0; Pos < File->Wrapper->FramesInfo.frames.size(); Pos++)
            {
                Text += "\t\t\t<frame";
                {
                    Text += " n=\"";
                    Text += to_string(Pos);
                    Text += '\"';
                }
                {
                    Text += " pts=\"";
                    seconds_to_timestamp(Text, File->Wrapper->FramesInfo.frames[Pos].pts / 1000000000.0, 6, true);
                    Text += '\"';
                }
                if (File->Wrapper->FramesInfo.frames[Pos].tc.HasValue())
                {
                    uint64_t tc_in_seconds = (File->Wrapper->FramesInfo.frames[Pos].tc.Hours * 3600)
                                           + (File->Wrapper->FramesInfo.frames[Pos].tc.Minutes * 60)
                                           + (File->Wrapper->FramesInfo.frames[Pos].tc.Seconds);
                    bool dropframe = File->Wrapper->FramesInfo.frames[Pos].tc.DropFrame;
                    uint8_t frame = File->Wrapper->FramesInfo.frames[Pos].tc.Frames;

                    Text += " tc=\"";
                    timecode_to_string(Text, tc_in_seconds, dropframe, frame);
                    Text += '\"';

                    if (File->Wrapper->FramesInfo.frames[Pos].tc_r)
                        Text += " tc_r=\"1\"";
                    else if (File->Wrapper->FramesInfo.frames[Pos].tc_nc == 2)
                        Text += " tc_nc=\"2\"";
                    else if (File->Wrapper->FramesInfo.frames[Pos].tc_nc == 1)
                        Text += " tc_nc=\"1\"";
                }

                if (File->Wrapper->FramesInfo.frames[Pos].st.HasValue())
                {
                    Text += ">\n";
                    Text += "\t\t\t\t<signalstats";
                    if (!isnan(File->Wrapper->FramesInfo.frames[Pos].st.SatAvg))
                    {
                        stringstream ss;
                        ss << File->Wrapper->FramesInfo.frames[Pos].st.SatAvg;
                        Text += " satavg=\"";
                        Text += ss.str();
                        Text += '\"';
                    }

                    if (File->Wrapper->FramesInfo.frames[Pos].st.SatHi != (uint16_t)-1)
                    {
                        Text += " sathigh=\"";
                        Text += to_string(File->Wrapper->FramesInfo.frames[Pos].st.SatHi);
                        Text += '\"';
                    }

                    if (File->Wrapper->FramesInfo.frames[Pos].st.SatMax != (uint16_t)-1)
                    {
                        Text += " satmax=\"";
                        Text += to_string(File->Wrapper->FramesInfo.frames[Pos].st.SatMax);
                        Text += '\"';
                    }
                    Text += "/>\n";
                    Text += "\t\t\t</frame>\n";
                }
                else
                    Text += "/>\n";
            }

            // Media footer
            Text += "\t\t</frames>\n";
            Text += "\t</media>\n";
            continue;
        }
        #endif

        // By Frame - For each line
        auto FrameNumber_Max = File->PerFrame.size() - 1;
        auto PerChange_Next = File->PerChange.begin();
        auto ShowFrames = true;
        queue<size_t> Captions_Partial[2]; // 0 = Out, 1 = In
        for (const auto& Frame : File->PerFrame)
        {
            string abst;
            auto AbstBf = abst_bf(Frame->AbstBf);
            if (AbstBf.HasAbsoluteTrackNumberValue())
                abst = to_string(AbstBf.AbsoluteTrackNumber());
            auto abst_r = AbstBf.Repeat();
            auto abst_nc = AbstBf.NonConsecutive();
            string tc;
            auto TimeCode = timecode(Frame);
            if (TimeCode.HasValue())
                timecode_to_string(tc, TimeCode.TimeInSeconds(), TimeCode.DropFrame(), TimeCode.Frames());
            auto tc_r = TimeCode.Repeat();
            int tc_nc = TimeCode.NonConsecutive();
            if (tc_nc && TimeCode.NonConsecutive_IsLess())
                tc_nc++;

            if (Frame->MoreData)
            {
                const auto MoreData = Frame->MoreData;
                size_t MoreData_Offset = sizeof(size_t);
                size_t MoreData_TotalSize = MoreData_Offset + *((size_t*)MoreData);
                while (MoreData_Offset < MoreData_TotalSize)
                {
                    auto Size = MoreData[MoreData_Offset++];
                    if (Size >= 0x80)
                        break;
                    auto Name = MoreData[MoreData_Offset++];
                    if (Name >= 0x80)
                        break;
                    switch (Name)
                    {
                    case MediaInfo_Event_Analysis_Frame_TimeCode:
                        if (Size >= 4)
                        {
                            uint32_t Value = *((int32_t*)(MoreData + MoreData_Offset));
                            uint32_t MoreFlags = 0;
                            bool IsAbst = false;
                            if (Size >= 8)
                            {
                                MoreFlags = *((int32_t*)(MoreData + MoreData_Offset + 4));
                                if (Size >= 9)
                                {
                                    IsAbst = MoreData[MoreData_Offset + 8] == 1;
                                }
                            }
                            timecode TimeCode(Value, MoreFlags);
                            if (IsAbst && abst.empty())
                            {
                                timecode_to_string(abst, TimeCode.TimeInSeconds(), TimeCode.DropFrame(), TimeCode.Frames());
                                abst_r = TimeCode.Repeat();
                                abst_nc = TimeCode.NonConsecutive();
                            }
                            else if (!IsAbst && tc.empty())
                            {
                                timecode_to_string(tc, TimeCode.TimeInSeconds(), TimeCode.DropFrame(), TimeCode.Frames());
                                tc_r = TimeCode.Repeat();
                                tc_nc = TimeCode.NonConsecutive();
                            }
                        }
                        break;
                    }
                    MoreData_Offset += Size;
                }
            }

            decltype(FrameNumber_Max) FrameNumber = &Frame - &*File->PerFrame.begin();
            auto ShowFrame = ShowFrames || FrameNumber == FrameNumber_Max || Frame_HasErrors(Frame) || abst_r || abst_nc || tc_r || tc_nc;

            if (ShowFrames)
            {
                ShowFrames = false;

                if (FrameNumber)
                    Text += "\t\t</frames>\n";

                const auto Change = *PerChange_Next;
                PerChange_Next++;

                auto no_sourceorcontrol_aud_Everywhere = true;
                const auto& Frame2_Max = PerChange_Next != File->PerChange.end() ? (File->PerFrame.begin() + (*PerChange_Next)->FrameNumber) : File->PerFrame.end();
                for (auto Frame2 = File->PerFrame.begin() + FrameNumber; Frame2 < Frame2_Max; ++Frame2)
                {
                    coherency_flags Coherency(*Frame2);
                    if (Coherency.no_pack_aud() || !Coherency.no_sourceorcontrol_aud())
                    {
                        no_sourceorcontrol_aud_Everywhere = false;
                        break;
                    }
                }

                if (!no_sourceorcontrol_aud_Everywhere)
                {
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
                                || ((*PerChange_Next)->FrameNumber && GetDvSpeedHasChanged(File->PerFrame[(*PerChange_Next)->FrameNumber - 1], File->PerFrame[(*PerChange_Next)->FrameNumber]))
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
                }
                Text += "\t\t<frames";
                {
                    auto FrameCount = (PerChange_Next != File->PerChange.end() ? (*PerChange_Next)->FrameNumber : (FrameNumber_Max + 1)) - FrameNumber;
                    Text += " count=\"";
                    Text += to_string(FrameCount);
                    Text += '\"';
                }
                {
                    auto TimeStamp_Begin = Frame->PTS / 1000000000.0; // FrameNumber / File->FrameRate;
                    Text += " pts=\"";
                    seconds_to_timestamp(Text, TimeStamp_Begin, 6, true);
                    Text += '\"';
                }
                {
                    auto TimeStamp_End = (PerChange_Next != File->PerChange.end() ? (*PerChange_Next)->PTS : (File->PerFrame.back()->PTS + File->PerFrame.back()->DUR)) / 1000000000.0; //(PerChange_Next != File->PerChange.end() ? (*PerChange_Next)->FrameNumber : (FrameNumber_Max + 1)) / File->FrameRate;
                    Text += " end_pts=\"";
                    seconds_to_timestamp(Text, TimeStamp_End, 6, true);
                    Text += '\"';
                }
                {
                    auto AbstBf = abst_bf(Frame->AbstBf);
                    if (AbstBf.HasAbsoluteTrackNumberValue())
                    {
                        Text += " abst=\"";
                        Text += to_string(AbstBf.AbsoluteTrackNumber());
                        Text += '\"';
                    }
                }
                {
                    auto FrameNumber = PerChange_Next != File->PerChange.end() ? (*PerChange_Next)->FrameNumber : (MediaInfo_int64u)File->PerFrame.size()-1;
                    if (FrameNumber < File->PerFrame.size())
                    {
                        auto AbstBf = abst_bf(File->PerFrame[FrameNumber]->AbstBf);
                        if (AbstBf.HasAbsoluteTrackNumberValue())
                        {
                            Text += " end_abst=\"";
                            Text += to_string(AbstBf.AbsoluteTrackNumber());
                            Text += '\"';
                        }
                    }
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
                if (Change->AudioBitDepth)
                {
                    Text += " audio_bitdepth=\"";
                    Text += to_string(Change->AudioBitDepth);
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
                if (no_sourceorcontrol_aud_Everywhere)
                {
                    Text += " no_sourceorcontrol_aud=\"1\"";
                }
                auto DvSpeed = GetDvSpeedIfNotPlayback(*Frame);
                if (DvSpeed != INT_MIN)
                {
                    Text += " speed=\"";
                    Text += to_string(DvSpeed);
                    Text += '\"';
                }
                if (Change->MoreData)
                {
                    const auto MoreData = Change->MoreData;
                    size_t MoreData_Offset = sizeof(size_t);
                    size_t MoreData_TotalSize = MoreData_Offset + *((size_t*)MoreData);
                    while (MoreData_Offset < MoreData_TotalSize)
                    {
                        auto Size = MoreData[MoreData_Offset++];
                        if (Size >= 0x80)
                            break;
                        auto Name = MoreData[MoreData_Offset++];
                        if (Name >= 0x80)
                            break;
                        switch (Name)
                        {
                        case MediaInfo_Event_Change_MoreData_Emphasis:
                            if (Size)
                            {
                                auto emphasis = MoreData[MoreData_Offset];
                                if (emphasis < 2)
                                {
                                    Text += " emphasis=\"";
                                    Text += emphasis ? "50/15 ms" : "off";
                                    Text += '\"';
                                }
                            }
                            break;
                        case MediaInfo_Event_Change_MoreData_ProgramNumber:
                            if (Size > 1)
                            {
                                auto program_number = to_hexstring(*((int16_t*)(MoreData + MoreData_Offset)));
                                if (program_number.size() == 6 && program_number[0] == '0' && program_number[1] == 'x' && program_number[2] == '0')
                                    program_number.erase(0, 3);
                                Text += " program_number=\"";
                                Text += program_number;
                                Text += '\"';
                            }
                            break;
                        }
                        MoreData_Offset += Size;
                    }
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
                auto TimeStamp = Frame->PTS / 1000000000.0;

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
                    seconds_to_timestamp(Text, TimeStamp, 6, true);
                    Text += '\"';
                }

                // Absolute track number + Blank flag
                if (!abst.empty())
                {
                    Text += " abst=\"";
                    Text += abst;
                    Text += '\"';
                }
                if (abst_r)
                {
                    Text += " abst_r=\"1\"";
                }
                if (abst_nc)
                {
                    Text += " abst_nc=\"1\"";
                }

                // TimeCode
                if (!tc.empty())
                {
                    Text += " tc=\"";
                    Text += tc;
                    Text += '\"';
                }
                if (tc_r)
                {
                    Text += " tc_r=\"1\"";
                }
                if (tc_nc)
                {
                    Text += " tc_nc=\"";
                    Text += '0' + tc_nc;
                    Text += '\"';
                }

                // RecDate/RecTime
                rec_date_time RecDateTime(Frame);
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
                    if (RecDateTime.NonConsecutive_IsLess())
                        Text += " rdt_nc=\"2\"";
                    else
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
                frame_seqn Seqn(Frame);
                if (Seqn.HasValue())
                {
                    Text += string(" seqn=\"") + uint4_to_hex4(Seqn.Value()) + "\"";
                }
                if (Seqn.Repeat())
                {
                    Text += " seqn_r=\"1\"";
                }
                if (Seqn.NonConsecutive())
                {
                    if (Seqn.NonConsecutive_IsLess())
                        Text += " seqn_nc=\"2\"";
                    else
                        Text += " seqn_nc=\"1\"";
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
                coherency_flags Coherency(Frame);
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
                        if (Frame->MoreData)
                        {
                            // Compute
                            computed_errors ComputedErrors;
                            set<int16u> Values;
                            for (int Dseq = 0; Dseq < Dseq_Size; Dseq++)
                            {
                                if (ComputedErrors.Compute(*Frame, Dseq))
                                {
                                    for (size_t i = 0; i < ComputedErrors.PerDseq.Audio_Errors_Values.size(); i++)
                                    {
                                        Values.insert(ComputedErrors.PerDseq.Audio_Errors_Values[i]);
                                    }
                                }
                            }
                            if (!Values.empty())
                            {
                                Text += " conceal_aud_type=\"2\" conceal_aud_value=\"";
                                for (set<int16u>::iterator Value = Values.begin(); Value != Values.end(); ++Value)
                                {
                                    if (Value != Values.begin())
                                        Text += ' ';
                                    Text += to_hexstring(*Value);
                                }
                                Text += '\"';
                            }
                        }
                    }
                    else if (Coherency.conceal_aud_l())
                    {
                        Text += " full_conceal_aud=\"l\"";
                    }
                    else if (Coherency.conceal_aud_r())
                    {
                        Text += " full_conceal_aud=\"r\"";
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
                                Aud_Element(Text, 5, ComputedErrors.PerDseq.Audio_Data_Total, ComputedErrors.PerDseq.Audio_Errors_Values);
                            Dseq_End(Text, 4);
                        }
                    }

                    // Display
                    if (!Coherency.full_conceal_vid())
                        Sta_Elements(Text, 4, ComputedErrors.Video_Sta_TotalPerSta, ComputedErrors.Video_Sta_EvenTotalPerSta);
                    if (!Coherency.full_conceal_aud())
                        Aud_Element(Text, 4, ComputedErrors.Audio_Data_Total, ComputedErrors.PerDseq.Audio_Errors_Values, ComputedErrors.Audio_Data_EvenTotal);

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
