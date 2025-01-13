/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include <vector>
#include <ostream>
#include "Common/Core.h"
using namespace std;
class file;
//---------------------------------------------------------------------------

return_value Output_Xml(Core::OutFile& Out, vector<file*>& PerFile, bitset<Option_Max> Options, ostream* Err = nullptr);

//---------------------------------------------------------------------------
#if defined(ENABLE_DECKLINK) || defined(ENABLE_SIMULATOR)
string decklink_pixelformat_to_string(uint8_t value);
#endif
