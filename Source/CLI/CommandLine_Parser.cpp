/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a Unlicense license that can
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
//---------------------------------------------------------------------------

//***************************************************************************
// Command line parser
//***************************************************************************

return_value Parse(Core &C, int argc, const char* argv_ansi[], const MediaInfoNameSpace::Char* argv[])
{
    bool ClearInput = false;

    for (int i = 1; i < argc; i++)
    {
             if (strcmp(argv_ansi[i], "--help") == 0 || strcmp(argv_ansi[i], "-h") == 0)
        {
            if (auto Value = Help(argv_ansi[0], true))
                return Value;
            ClearInput = true;
        }
        else if (strcmp(argv_ansi[i], "--version") == 0)
        {
            if (auto Value = NameVersion())
                return Value;
            ClearInput = true;
        }
        else
            C.Inputs.push_back(argv[i]);
    }

    if (!ClearInput && C.Inputs.empty())
    {
        if (auto Value = Help(argv_ansi[0]))
            return Value;
        return ReturnValue_ERROR;
    }

    if (ClearInput)
        C.Inputs.clear();

    return ReturnValue_OK;
}

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
