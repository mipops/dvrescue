/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a Unlicense license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include "Common/Config.h"
class Core;
//---------------------------------------------------------------------------

//***************************************************************************
// Command line parser
//***************************************************************************

return_value Parse(Core &C, int argc, const char* argv[]);
