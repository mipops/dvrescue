/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <cmath>
//***************************************************************************
// Class SignalStats
//***************************************************************************

struct decklink_frame;
class SignalStats {
public:
    struct Stats {
        double SatAvg = -NAN;
        uint16_t SatHi = -1;
        uint16_t SatMax = -1;

        bool HasValue() const
        {
            return !std::isnan(SatAvg) ||
                   SatHi != (uint16_t)-1 ||
                   SatMax != (uint16_t)-1;
        };
    };

    static Stats ComputeStats(const decklink_frame* Frame);

private:
    int Width;
    int Height;
};