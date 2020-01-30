/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include <vector>
#include <ostream>
using namespace std;
class file;
//---------------------------------------------------------------------------

return_value Output_Webvtt(ostream& Out, vector<file*>& PerFile, ostream* Err = nullptr);
