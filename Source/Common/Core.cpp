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
#include "ZenLib/Ztring.h"
#include <future>
#include <mutex>
#include <thread>
#include <csignal>
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
return_value Core::Process()
{
    return_value ToReturn = ReturnValue_OK;

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
    for (auto &future : futures) {
        if (auto ToReturn2 = future.get())
            ToReturn = ToReturn2;
    }
    if (Device_Command)
        return ToReturn;

    if (!Merge_Out.empty())
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
    #if !defined(_WIN32) && !defined(WIN32)
    case SIGPIPE:
    #endif
        for (auto& File : PerFile)
            File->Terminate();
        break;
    default:
        ;
    }
}
