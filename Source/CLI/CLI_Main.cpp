/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a Unlicense license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "CLI/CommandLine_Parser.h"
#include "Common/Core.h"
#include <iostream>
//---------------------------------------------------------------------------

//***************************************************************************
// Main
//***************************************************************************

int main(int argc, const char* argv[])
{
    // Configure core
    Core C;
    if (auto ReturnValue = Parse(C, argc, argv))
        return ReturnValue;

    // Process
    C.Process();

    // Output
    cout << C.OutputXml();
    
    return ReturnValue_OK;
}
//---------------------------------------------------------------------------

