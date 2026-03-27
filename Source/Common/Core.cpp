/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/Core.h"
#include "Common/ProcessFile.h"
#include "Common/Output_Captions_Decode.h"
#include "Common/Output_Captions_Scc.h"
#include "Common/Output_Xml.h"
#include "Common/Output_Webvtt.h"
#include "Common/MergeTs.h"
#include "ZenLib/Ztring.h"
#include <future>
#include <mutex>
#include <thread>
#include <csignal>
#include <chrono>
#include <algorithm>
using namespace ZenLib;
using namespace std;

//---------------------------------------------------------------------------
void Handle_Signal(int);

//---------------------------------------------------------------------------

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Core::Core()
{
    MediaInfo::Option_Static(__T("ParseSpeed"), __T("1.000"));
    MediaInfo::Option_Static(__T("Demux"), __T("container"));
    MediaInfo::Option_Static(__T("AcceptSignals"), __T("0"));

    std::signal(SIGINT, Handle_Signal);
    #if !defined(_WIN32) && !defined(WIN32)
    std::signal(SIGPIPE, Handle_Signal);
    #endif
}

Core::~Core()
{
    std::signal(SIGINT, SIG_DFL);
    #if !defined(_WIN32) && !defined(WIN32)
    std::signal(SIGPIPE, SIG_DFL);
    #endif
    PerFile_Clear();
}

//***************************************************************************
// Process
//***************************************************************************

//---------------------------------------------------------------------------
vector<file*>   PerFile;
atomic_bool     Terminate = false, Terminated = false;
return_value Core::Process()
{
    return_value ToReturn = ReturnValue_OK;

    // MPEG-TS merge path: if merge is requested and all inputs are MPEG-TS,
    // use the TS packet-level merge instead of the DV block-level merge.
    bool TsMergeUsed = false;
    if (!Merge_Out.empty() && !Merge_OutputFileNames.empty() && Inputs.size() >= 2)
    {
        bool AllMpegTs = true;
        for (const auto& Input : Inputs)
        {
            if (!ts_merge::IsMpegTs(Ztring(Input).To_Local()))
            {
                AllMpegTs = false;
                break;
            }
        }
        if (AllMpegTs)
        {
            std::vector<std::string> InputPaths;
            for (const auto& Input : Inputs)
                InputPaths.push_back(Ztring(Input).To_Local());

            ostream* MergeLog = MergeInfo_Out ? MergeInfo_Out : (Err ? Err : &cerr);
            TsMergeUsed = ts_merge::Process(InputPaths, Merge_OutputFileNames, MergeLog, Verbosity);

            if (!TsMergeUsed && Err)
                *Err << "Warning: MPEG-TS merge failed, falling back to normal analysis." << endl;
        }
    }

    // Analyze files (asynchronous)
    PerFile_Clear();
    PerFile.reserve(Inputs.size());
    std::vector<std::future<return_value>> futures;
    for (const auto& Input : Inputs)
    {
        PerFile.push_back(new file());
        futures.emplace_back(std::async(std::launch::async, [](size_t index, const String& Input) {
            return PerFile[index]->Parse(Input);
            }, PerFile.size()-1, Input));
    }

    while (!futures.empty())
    {
        auto it = futures.begin();
        if (it->wait_for(100ms) != std::future_status::timeout)
        {
           if (auto ToReturn2 = it->get())
                ToReturn = ToReturn2;

            futures.erase(it);
        }

        if (Terminate && !Terminated)
        {
            for (auto& File : PerFile)
                File->Terminate();

            Terminated = true;
        }
    }

    if (Device_Command)
        return ToReturn;

    if (!Merge_Out.empty() && !TsMergeUsed && !PerFile.empty())
    {
        PerFile[0]->Merge_Finish();
        if (!XmlFile)
            return ToReturn;
    }

    // Set output defaults
    if (!XmlFile)
        XmlFile = Out;

    // XML
    if (XmlFile)
    {
        if (auto ToReturn2 = Output_Xml(*XmlFile, PerFile, Options, Err))
            ToReturn = ToReturn2;
    }

    // WebVTT
    if (WebvttFile)
    {
        if (auto ToReturn2 = Output_Webvtt(*WebvttFile, PerFile, Err))
            ToReturn = ToReturn2;
    }

    // Closed Captions
    if (!CaptionsFileNames.empty())
    {
        // SCC
        auto SccFileName = CaptionsFileNames.find(Caption_Scc);
        if (SccFileName != CaptionsFileNames.end())
        {
            if (auto ToReturn2 = Output_Captions_Scc(SccFileName->second, OffsetTimeCode, PerFile, Err))
                ToReturn = ToReturn2;
        }

        // Decode (Screen or SRT)
        auto ScreenFileName = CaptionsFileNames.find(Caption_Screen);
        auto SrtFileName = CaptionsFileNames.find(Caption_Srt);
        if (ScreenFileName != CaptionsFileNames.end() || SrtFileName != CaptionsFileNames.end())
        {
            if (auto ToReturn2 = Output_Captions_Caption(ScreenFileName != CaptionsFileNames.end() ? ScreenFileName->second : string(), SrtFileName != CaptionsFileNames.end() ? SrtFileName->second : string(), OffsetTimeCode, PerFile, Err))
                ToReturn = ToReturn2;
        }
    }

    return ToReturn;
}

//---------------------------------------------------------------------------
float Core::State ()
{
    size_t Total = 0;
    for (const auto& File : PerFile)
        Total += File->MI.State_Get();
    return (((float)Total)/PerFile.size()/10000);
}

//***************************************************************************
// PerFile
//***************************************************************************

//---------------------------------------------------------------------------
void Core::PerFile_Clear()
{
    for (const auto& File : PerFile)
        delete File;
    PerFile.clear();
}

//***************************************************************************
// Signals handler
//***************************************************************************

//---------------------------------------------------------------------------
void Handle_Signal(int Signal)
{
    switch (Signal)
    {
    case SIGINT:
        if (Terminate)
        {
            // Second Ctrl-C: restore default handler so a third one kills immediately
            std::signal(SIGINT, SIG_DFL);
            return;
        }
        Terminate = true;
        // Re-register so that a second Ctrl-C is caught (some platforms reset to SIG_DFL)
        std::signal(SIGINT, Handle_Signal);
        break;
    #if !defined(_WIN32) && !defined(WIN32)
    case SIGPIPE:
        Terminate = true;
        break;
    #endif
    default:
        ;
    }
}
