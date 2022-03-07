/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "CLI/CommandLine_Parser.h"
#include "CLI/CLI_Help.h"
#include "Common/Core.h"
#include "TimeCode.h"
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
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
static const char* ExtensionToCatpionKind_Array[]
{
    "scc",
    "screen",
    "srt",
};
static_assert(Caption_Unknown == sizeof(ExtensionToCatpionKind_Array) / sizeof(const char*), "");
static caption_kind ExtensionToCatpionKind(const char* Value)
{
    for (auto i = 0; i < Caption_Unknown; i++)
        if (!strcmp(Value, ExtensionToCatpionKind_Array[i]))
            return (caption_kind)i;
    return Caption_Unknown;
}
static string CatpionKindsString()
{
    string Value;
    for (auto i = 0; i < Caption_Unknown; i++)
    {
        Value += "'";
        Value += ExtensionToCatpionKind_Array[i];
        Value += "', ";
    }
    Value.resize(Value.size() - 2);
    return Value;
}

//***************************************************************************
// Command line parser
//***************************************************************************

//---------------------------------------------------------------------------
return_value Parse(Core &C, int argc, const char* argv_ansi[], const MediaInfoNameSpace::Char* argv[])
{
    return_value ReturnValue = ReturnValue_OK;
    bool ClearInput = false;
    caption_kind CaptionKind = Caption_Unknown;

    // Commands in priority
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv_ansi[i], "--verbosity") || !strcmp(argv_ansi[i], "-v"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing verbosity level after " << argv_ansi[i-1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            if (!strcmp(argv_ansi[i], "debug") && C.Err)
            {
                Verbosity = 10;

                *C.Err << "Debug: called with";
                for (int i = 0; i < argc; i++)
                {
                    *C.Err << ' ' << '"' << argv_ansi[i] << '"';
                }
                *C.Err << '.' << endl;
                break;
            }
        }
    }
    
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
        else if (!strcmp(argv_ansi[i], "--caption-presence-change"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing value after " << argv_ansi[i - 1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            if (!strcmp(argv_ansi[i], "n"))
            {
                C.Options.set(Option_CaptionPresenceChange, false);
            }
            else if (!strcmp(argv_ansi[i], "y"))
            {
                C.Options.set(Option_CaptionPresenceChange, true);
            }
            else
            {
                if (C.Err)
                    *C.Err << "Error: invalid value after " << argv_ansi[i - 1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
        }
        else if (!strcmp(argv_ansi[i], "--cc-format"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing Closed Captions output format after " << argv_ansi[i-1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            CaptionKind = ExtensionToCatpionKind(argv_ansi[i]);
            if (CaptionKind == Caption_Unknown)
            {
                if (C.Err)
                    *C.Err << "Error: unknown Closed Captions output format " << argv_ansi[i] << " (not 'scc', 'screen', 'srt').\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
        }
        else if (!strcmp(argv_ansi[i], "--cc-tc"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing Closed Captions output format after " << argv_ansi[i-1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            if (!strcmp(argv_ansi[i], "dv"))
            {
                C.OffsetTimeCode = new TimeCode();
                continue;
            }
            C.OffsetTimeCode = new TimeCode();
            if (strcmp(argv_ansi[i], "dv") && C.OffsetTimeCode->FromString(argv_ansi[i])) // if "dv", DV time code is used, else the string. Frame rate is provided later by DV stream
            {
                if (C.Err)
                    *C.Err << "Error: invalid Time Code format " << argv_ansi[i] << " (not 'HH:MM:SS;FF').\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
        }
        else if (!strcmp(argv_ansi[i], "--cc-output") || !strcmp(argv_ansi[i], "-c"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing Closed Captions output file name after " << argv_ansi[i-1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            caption_kind CurrentCaptionKind;
            auto CurrentFileName = string(argv_ansi[i]);
            if (CaptionKind == Caption_Unknown)
            {
                auto DotPos = CurrentFileName.rfind('.');
                if (DotPos != string::npos)
                    CurrentCaptionKind = ExtensionToCatpionKind(CurrentFileName.c_str() + DotPos + 1);
                else
                    CurrentCaptionKind = Caption_Unknown;
            }
            else
            {
                CurrentCaptionKind = CaptionKind;
                CaptionKind = Caption_Unknown;
            }
            C.CaptionsFileNames[CurrentCaptionKind] = move(CurrentFileName);
        }
        else if (!strcmp(argv_ansi[i], "--csv"))
        {
            MergeInfo_Format = 1;
        }
        else if (!strcmp(argv_ansi[i], "--merge") || !strcmp(argv_ansi[i], "-m"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing merged output file name after " << argv_ansi[i - 1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            Merge_OutputFileName = move(argv_ansi[i]);
        }
        else if (!strcmp(argv_ansi[i], "--merge-log"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing merge info output file name after " << argv_ansi[i-1] << ".\n";
                return ReturnValue_ERROR;
            }
            if (Verbosity == 10)
                *C.Err << "Debug: opening (out, trunc) \"" << argv_ansi[i] << "\"..." << endl;
            auto File = new ofstream(argv_ansi[i], ios_base::trunc);
            if (Verbosity == 10)
                *C.Err << "Debug: opening (out, trunc) \"" << argv_ansi[i] << "\"... Done." << endl;
            if (!File->is_open())
            {
                if (C.Err)
                    *C.Err << "Error: can not open " << argv_ansi[i] << " for writing.\n";
                delete File;
                return ReturnValue_ERROR;
            }
            else
                MergeInfo_OutputFileName = argv_ansi[i];
        }
        else if (!strcmp(argv_ansi[i], "--use-abst"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing value (y or n) after " << argv_ansi[i-1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            switch (argv_ansi[i][0])
            {
                case 'n':
                    UseAbst = 0;
                    break;
                case 'y':
                    UseAbst = 1;
                    break;
                default:
                    if (C.Err)
                        *C.Err << "Error: invalid value " << argv_ansi[i] << " (must be y or n).\n";
                    ReturnValue = ReturnValue_ERROR;
                    continue;
            }
        }
        else if (!strcmp(argv_ansi[i], "--verbosity") || !strcmp(argv_ansi[i], "-v"))
        {
            if (++i >= argc)
            {
                if (C.Err)
                    *C.Err << "Error: missing verbosity level after " << argv_ansi[i-1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            switch (argv_ansi[i][0])
            {
                case '0':
                case '5':
                case '7':
                case '9':
                    Verbosity = argv_ansi[i][0] - '0';
                    break;
                case 'd':
                    if (!strcmp(argv_ansi[i], "debug") && C.Err)
                    {
                        Verbosity = 10;
                        break;
                    }
                    //Fallthrough
                default:
                    if (C.Err)
                        *C.Err << "Error: invalid verbosity " << argv_ansi[i] << " (must be 0, 5, 7, 9 or debug).\n";
                    ReturnValue = ReturnValue_ERROR;
                    continue;
            }
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
                if (C.Err)
                    *C.Err << "Error: missing WebVTT output file name after " << argv_ansi[i-1] << ".\n";
                ReturnValue = ReturnValue_ERROR;
                continue;
            }
            if (Verbosity == 10)
                *C.Err << "Debug: opening (out, trunc) \"" << argv_ansi[i] << "\"..." << endl;
            auto File = new ofstream(argv_ansi[i], ios_base::trunc);
            if (Verbosity == 10)
                *C.Err << "Debug: opening (out, trunc) \"" << argv_ansi[i] << "\"... Done." << endl;
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
            if (Verbosity == 10)
                *C.Err << "Debug: opening (out, trunc) \"" << argv_ansi[i] << "\"..." << endl;
            auto File = new ofstream(argv_ansi[i], ios_base::trunc);
            if (Verbosity == 10)
                *C.Err << "Debug: opening (out, trunc) \"" << argv_ansi[i] << "\"... Done." << endl;
            if (!File->is_open())
            {
                if (C.Err)
                    *C.Err << "Error: can not open " << argv_ansi[i] << " for writing.\n";
                delete File;
                return ReturnValue_ERROR;
            }
            else
                C.XmlFile = File;
        }
        else if (!strncmp(argv_ansi[i], "--", 2))
        {
            //Library options, we may accept either --option value or --option=value
            String Option(argv[i] + 2);
            auto EqualPos = Option.find('=');
            String Value;
            if (EqualPos != string::npos)
            {
                Value.assign(Option, EqualPos + 1);
                Option.resize(EqualPos);
                EqualPos = 0;
            }
            else
            {
                if (++i >= argc)
                {
                    if (C.Err)
                        *C.Err << "Error: missing value after " << argv_ansi[i - 1] << ".\n";
                    return ReturnValue_ERROR;
                }
                Value.assign(argv[i]);
                EqualPos = 1;
            }
            String Result = MediaInfoLib::MediaInfo::Option_Static(Option, Value);
            if (C.Err && !Result.empty())
                *C.Err << "Warning: issue with " << argv_ansi[i - EqualPos];
        }
        else
        {
            if (C.WebvttFile || C.XmlFile || !Merge_OutputFileName.empty())
            {
                if (C.Err)
                    *C.Err << "Error: in order to avoid mistakes, provide output file names after input file names.\n";
                return ReturnValue_ERROR;
            }
            C.Inputs.push_back(argv[i]);
            Merge_InputFileNames.push_back(argv_ansi[i]);
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

    if (!C.CaptionsFileNames.empty())
    {
        auto FileNameWithUnknownFormat = C.CaptionsFileNames.find(Caption_Unknown);
        if (FileNameWithUnknownFormat != C.CaptionsFileNames.end())
        {
            if (CaptionKind != Caption_Unknown)
            {
                auto ToMove = FileNameWithUnknownFormat->second;
                C.CaptionsFileNames.erase(FileNameWithUnknownFormat);
                C.CaptionsFileNames[CaptionKind] = ToMove;
            }
            else
            {
                if (C.Err)
                    *C.Err << "Error: Closed Captions output format not detectable, use a known extension (" << CatpionKindsString() << ") or --cc-format option.\n";
                return ReturnValue_ERROR;
            }
        }

        if (C.Inputs.size() >= 2)
        {
            if (C.Err)
                *C.Err << "Error: Closed Captions output is possible only with one input file.\n";
            return ReturnValue_ERROR;
        }
    }

    if (MergeInfo_Format)
    {
        if (Merge_OutputFileName.empty())
        {
            if (C.Err)
                *C.Err << "Error: CSV format is available only for merge feature.\n";
            return ReturnValue_ERROR;
        }
        Verbosity = 9;
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