/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include <vector>
#include <string>
#include "Common/Core.h"
using namespace std;
class file;
class TimeCode;
//---------------------------------------------------------------------------

return_value Output_Captions_Scc(const Core::OutFile& Out, const TimeCode* OffsetTimeCode, vector<file*>& PerFile, ostream* Err = nullptr);
