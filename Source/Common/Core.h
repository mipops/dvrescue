/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include "Common/ProcessFile.h"
#include <ostream>
#include <vector>
using namespace MediaInfoNameSpace;
using namespace std;
//---------------------------------------------------------------------------

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
    ostream*        WebvttFile = nullptr;
    ostream*        XmlFile = nullptr;
    ostream*        Out = nullptr;
    ostream*        Err = nullptr;

    // Process
    return_value    Process();
    float           State();

protected:
    vector<file*>   PerFile;
    void            PerFile_Clear();
};
