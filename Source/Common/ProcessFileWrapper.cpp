/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

#include "Common/ProcessFileWrapper.h"
#include "Common/ProcessFile.h"

#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
#include "Common/Output_Mkv.h"
#include "Common/Merge.h"

#include <iostream>
#endif

using namespace std;

//---------------------------------------------------------------------------
FileWrapper::FileWrapper(file* File) : File(File)
{
}

//---------------------------------------------------------------------------
#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
FileWrapper::FileWrapper(int Width, int Height, int Framerate_Num, int Framerate_Den, int SampleRate, int Channels, bool Has_Timecode) : File(nullptr)
{
    IsMatroska = true;
    for (string OutputFile : Merge_OutputFileNames)
    {
        matroska_output Output;
        if (OutputFile == "-")
          Output.Writer = new matroska_writer(&cout, Width, Height, Framerate_Num, Framerate_Den, Has_Timecode);
        else
        {
            Output.Output = new ofstream(OutputFile, ios_base::binary | ios_base::trunc);
            Output.Writer = new matroska_writer(Output.Output, Width, Height, Framerate_Num, Framerate_Den, Has_Timecode);
        }
        Outputs.push_back(Output);
    }

    FramesInfo.video_width = Width;
    FramesInfo.video_height = Height;
    FramesInfo.video_rate_num = Framerate_Num;
    FramesInfo.video_rate_den = Framerate_Den;
    FramesInfo.audio_rate = SampleRate;
    FramesInfo.audio_channels = Channels;
}

//---------------------------------------------------------------------------
FileWrapper::~FileWrapper()
{
    if (IsMatroska)
    {
        for (matroska_output Output: Outputs)
        {
            if (Output.Writer)
            {
                Output.Writer->close(Output.Output);
                delete Output.Writer;
            }

            if (Output.Output)
            {
                Output.Output->close();
                delete Output.Output;
            }
        }
        Outputs.clear();
    }
}
#endif

//---------------------------------------------------------------------------
void FileWrapper::Parse_Buffer(const uint8_t *Buffer, size_t Buffer_Size)
{
    #if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
    if (IsMatroska)
    {
        decklink_frame* Frame=(decklink_frame*)Buffer;
        for (matroska_output& Output : Outputs)
        {
            Output.Writer->write_frame((const char*)Frame->Video_Buffer, Frame->Video_Buffer_Size,
                                       (const char*)Frame->Audio_Buffer, Frame->Audio_Buffer_Size, Frame->TC);
        }

        bool TimeCode_Repeat = false;
        bool TimeCode_NonConsecutive = false;
        bool TimeCode_NonConsecutive_IsLess = false;
        if (Frame->TC.HasValue() && FramesInfo.frames.size() && FramesInfo.frames.back().tc.HasValue())
        {
            TimeCode_Repeat = Frame->TC == FramesInfo.frames.back().tc;
            if (!TimeCode_Repeat)
            {
                TimeCode Previous = FramesInfo.frames.back().tc;
                TimeCode Current = Frame->TC;
                Previous.FramesPerSecond = (FramesInfo.video_rate_num / FramesInfo.video_rate_num) + (FramesInfo.video_rate_num % FramesInfo.video_rate_num);
                Previous.FramesPerSecond_Is1001 = FramesInfo.video_rate_den == 1001;
                Current.FramesPerSecond = Previous.FramesPerSecond;
                Current.FramesPerSecond_Is1001 = Previous.FramesPerSecond_Is1001;

                TimeCode_NonConsecutive = Current.ToFrames() != Previous.ToFrames() + 1;
                if (TimeCode_NonConsecutive)
                    TimeCode_NonConsecutive_IsLess = Current.ToFrames() < Previous.ToFrames();
            }
        }

        double FrameRate = (double)FramesInfo.video_rate_num / FramesInfo.video_rate_den;
        double ElapsedTime = (double)FramesInfo.frames.size() / FrameRate;
        FramesInfo.frames.push_back(decklink_framesinfo::frame {
            Frame->TC,
            TimeCode_Repeat,
            (uint8_t)(TimeCode_NonConsecutive ? (TimeCode_NonConsecutive_IsLess ? 2 : 1) : 0),
            ElapsedTime * 1000000000.0,
            1.0 / FrameRate
        });

        if (MergeInfo_Format == 1)
        {
            if (!FrameCount)
            {
                cout << "FramePos,abst,abst_r,abst_nc,tc,tc_r,tc_nc,rdt,rdt_r,rdt_nc,rec_start,rec_end,Used,Status,Comments,BlockErrors,BlockErrors_Even,IssueFixed"
                     << (Verbosity > 5 ? ",SourceSpeed,FrameSpeed,InputPos,OutputPos" : "")
                     << (ShowFrames_Intermediate ? ",RewindStatus" : "")
                     << endl;
            }

            cout << FrameCount++ // framePos
                 << "," // abst
                    "," // abst_r
                    "," // abst_nc
                    "," << (Frame->TC.HasValue() ? Frame->TC.ToString() : "XX:XX:XX:XX") // tc
                 << "," << (TimeCode_Repeat ? "1" : "") // tc_r
                 << "," << (TimeCode_NonConsecutive ? (TimeCode_NonConsecutive_IsLess ? "2" : "1") : "") // tc_nc
                 << "," // rdt
                    "," // rdt_r
                    "," // rdt_nc
                    "," // rec_start
                    "," // rec_end
                    "," // Used
                    "," // Status
                    "," // Comments
                    "," // BlockErrors
                    "," // BlocksErrors_Even
                    "," // IssueFixed
                 << (Verbosity > 5 ?
                        "," // SourceSpeed
                        "," // FrameSpeed
                        "," // InputPos
                        "," // OutputPos
                        : ""
                    )
                    << (ShowFrames_Intermediate ?
                        "," // RewindStatus
                        : ""
                    )
                 << endl;
        }
        else
            cerr << "\33[2K\rCapture frame " << ++FrameCount << ", press " << (InControl ? "q" : "ctrl+c") << " to stop.";
        return;
    }
    #endif

    if (File)
    {
        File->Speed_After = File->Capture->GetSpeed();
        File->Parse_Buffer(Buffer, Buffer_Size);
    }
}
