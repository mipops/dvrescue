/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include "Common/ProcessFile.h"
#include <map>
#include <ostream>
#include <string>
#include <vector>
using namespace MediaInfoNameSpace;
using namespace std;
class TimeCode;
//---------------------------------------------------------------------------

//***************************************************************************
// Enums
//***************************************************************************

enum caption_kind
{
    Caption_Scc,
    Caption_Screen,
    Caption_Srt,
    Caption_Unknown,
};

//***************************************************************************
// Class core
//***************************************************************************

class Core
{
public:
    // Constructor/Destructor
    Core();
    ~Core();

    // Input
    vector<String>  Inputs;
    map<caption_kind, string> CaptionsFileNames; // We don't directly open an ostream because file name may change if cc are not same and/or 2nd field and/or no cc
    TimeCode*       OffsetTimeCode = nullptr;
    ostream*        WebvttFile = nullptr;
    ostream*        XmlFile = nullptr;
    ostream*        Out = nullptr;
    ostream*        Err = nullptr;

    // Process
    return_value    Process();
    float           State();

protected:
    void            PerFile_Clear();
};
