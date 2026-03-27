/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
//---------------------------------------------------------------------------

//***************************************************************************
// MPEG-TS packet-level merge for HDV streams
//
// Multiple captures of the same HDV tape produce nearly identical MPEG-TS
// packet sequences. This merger aligns packets sequentially across inputs,
// detects errors via the Transport Error Indicator (TEI) bit and continuity
// counter (CC) tracking, and selects the best version of each packet for
// the output.
//***************************************************************************

class ts_merge
{
public:
    struct stats
    {
        size_t TotalPackets = 0;
        size_t ErrorPackets = 0;     // Packets with TEI set in best input
        size_t RecoveredPackets = 0; // Packets recovered from alternate input
        size_t CcErrors = 0;        // Continuity counter discontinuities in output
        std::vector<size_t> InputErrorCounts; // TEI count per input
        std::vector<size_t> InputCcErrors;    // CC discontinuity count per input
        std::vector<size_t> InputUsedCounts;  // Packets selected from each input
    };

    // Run the merge. Returns true on success.
    // inputs: list of MPEG-TS file paths
    // outputs: list of output file paths (typically one)
    // log: stream for merge info output (nullptr to suppress)
    // verbosity: 0=silent, 5=summary, 7+=per-packet detail
    static bool Process(const std::vector<std::string>& inputs,
                        const std::vector<std::string>& outputs,
                        std::ostream* log,
                        uint8_t verbosity);

    // Check if a file looks like MPEG-TS (starts with sync byte 0x47)
    static bool IsMpegTs(const std::string& filename);
};
