/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "CLI/CLI_Help.h"
#include <iostream>
using namespace std;
//---------------------------------------------------------------------------

//***************************************************************************
// Help
//***************************************************************************

//---------------------------------------------------------------------------
return_value Help(const char* Name, bool Full)
{
    cout <<
    "Usage: \"" << Name << " [-Options...] FileName1 [Filename2...]\"\n";
    if (!Full)
    {
        cout << "\"" << Name << " --help\" for displaying more information.\n";
        return ReturnValue_OK;
    }
    cout << "\n"
    "Options:\n"
    "--help, -h         Display this help and exit.\n"
    "--version          Display DVRescue version and exit.\n"
    ;
    
    return ReturnValue_OK;
}

//---------------------------------------------------------------------------
return_value NameVersion()
{
    cout <<
    NameVersion_Text() << ".\n"
    ;

    return ReturnValue_OK;
}

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
const char* NameVersion_Text()
{
    return
        "DVRescue v." Program_Version " by MIPoPS"
        ;
}
