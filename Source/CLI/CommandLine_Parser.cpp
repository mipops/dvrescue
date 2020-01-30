/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "CLI/CommandLine_Parser.h"
#include "CLI/CLI_Help.h"
#include "Common/Core.h"
#if defined(UNICODE) && defined(_WIN32)
struct IUnknown; // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected" with old SDKs and /permisive-
#include <windows.h>
#else
#include <ZenLib/Ztring.h>
#endif
#include <fstream>
using namespace std;
//---------------------------------------------------------------------------

//***************************************************************************
// Command line parser
//***************************************************************************

//---------------------------------------------------------------------------
return_value Parse(Core &C, int argc, const char* argv_ansi[], const MediaInfoNameSpace::Char* argv[])
{
    return_value ReturnValue = ReturnValue_OK;
    bool ClearInput = false;

    for (int i = 1; i < argc; i++)
    {
             if (!strcmp(argv_ansi[i], "--help") || !strcmp(argv_ansi[i], "-h"))
        {
            if (!C.Out)
                return ReturnValue_ERROR;
            if (auto Value = Help(*C.Out, argv_ansi[0], true))
                return Value;
            ClearInput = true;
        }
        else if (!strcmp(argv_ansi[i], "--version"))
        {
            if (!C.Out)
                return ReturnValue_ERROR;
            if (auto Value = NameVersion(*C.Out))
                return Value;
            ClearInput = true;
        }
        else if (!strcmp(argv_ansi[i], "--webvtt-output") || !strcmp(argv_ansi[i], "-s"))
        {
            if (++i >= argc)
            {
                if (!C.Err)
                    *C.Err << "Error: missing WebVTT output file name after " << argv_ansi[i-1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            auto File = new ofstream(argv_ansi[i], ios_base::trunc);
            if (!File->is_open())
            {
                if (C.Err)
                    *C.Err << "Error: can not open " << argv_ansi[i] << " for writing.\n";
                delete File;
                return ReturnValue_ERROR;
            }
            else
                C.WebvttFile = File;
        }
        else if (!strcmp(argv_ansi[i], "--xml-output") || !strcmp(argv_ansi[i], "-x"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing XML output file name after " << argv_ansi[i-1] << ".\n";
                return ReturnValue_ERROR;
            }
            auto File = new ofstream(argv_ansi[i], ios_base::trunc);
            if (!File->is_open())
            {
                if (!C.Err)
                    *C.Err << "Error: can not open " << argv_ansi[i] << " for writing.\n";
                delete File;
                return ReturnValue_ERROR;
            }
            else
                C.XmlFile = File;
        }
        else
        {
            if (C.WebvttFile || C.XmlFile)
            {
                if (C.Err)
                    *C.Err << "Error: in order to avoid mistakes, provide output file names after input file names.\n";
                return ReturnValue_ERROR;
            }
            C.Inputs.push_back(argv[i]);
        }
    }

    if (!ClearInput && C.Inputs.empty())
    {
        if (!C.Out)
            return ReturnValue_ERROR;
        if (auto Value = Help(*C.Out, argv_ansi[0]))
            return Value;
        return ReturnValue_ERROR;
    }

    if (C.WebvttFile && C.Inputs.size() >= 2)
    {
        if (C.Err)
            *C.Err << "Error: WebVTT output is possible only with one input file.\n";
        return ReturnValue_ERROR;
    }

    if (ClearInput)
        C.Inputs.clear();

    return ReturnValue;
}

//---------------------------------------------------------------------------
return_value Parse(Core &C, int argc, const char* argv_ansi[])
{
    //Get command line args in main()
#ifdef UNICODE
#ifdef _WIN32
    LPCWSTR* argv = (LPCWSTR*)CommandLineToArgvW(GetCommandLineW(), &argc);
#else //WIN32
    std::vector<MediaInfoNameSpace::String> argv_Temp;
    for (int i = 0; i < argc; i++)
    {
        ZenLib::Ztring FileName;
        FileName.From_Local(argv_ansi[i]);
        argv_Temp.push_back(FileName);
    }
    auto argv = new const MediaInfoNameSpace::Char*[argc];
    for (int i = 0; i < argc; i++)
    {
        argv[i] = argv_Temp[i].c_str();
    }
#endif //WIN32
#else //UNICODE
    auto argv = argv_ansi;
#endif //UNICODE

    return_value ReturnValue = Parse(C, argc, argv_ansi, argv);

    // Manage memory
#ifdef UNICODE
#ifdef _WIN32
    LocalFree(argv);
#else //WIN32
    delete[] argv;
#endif //WIN32
#endif //UNICODE

    return ReturnValue;
}

//---------------------------------------------------------------------------
void Clean(Core& C)
{
    // We previously set some output file pointers, deleting them
    if (C.WebvttFile != C.Out)
        delete C.WebvttFile;
    if (C.XmlFile != C.Out)
        delete C.XmlFile;
}