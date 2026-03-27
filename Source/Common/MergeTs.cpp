/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/MergeTs.h"
#include <algorithm>
#include <cstring>
#include <iomanip>
//---------------------------------------------------------------------------

//***************************************************************************
// Constants
//***************************************************************************

static const size_t TS_PACKET_SIZE = 188;
static const uint8_t TS_SYNC_BYTE = 0x47;
static const size_t PID_COUNT = 8192; // 13-bit PID space

//***************************************************************************
// Helpers
//***************************************************************************

namespace
{
    struct ts_header
    {
        bool    tei;    // Transport Error Indicator
        bool    pusi;   // Payload Unit Start Indicator
        uint16_t pid;   // Packet Identifier (13 bits)
        uint8_t  cc;    // Continuity Counter (4 bits)
        uint8_t  afc;   // Adaptation Field Control (2 bits)
        bool     valid; // Sync byte was 0x47

        ts_header() : tei(false), pusi(false), pid(0x1FFF), cc(0), afc(0), valid(false) {}
    };

    ts_header parse_header(const uint8_t* pkt)
    {
        ts_header h;
        h.valid = (pkt[0] == TS_SYNC_BYTE);
        if (!h.valid)
            return h;
        h.tei  = (pkt[1] >> 7) & 1;
        h.pusi = (pkt[1] >> 6) & 1;
        h.pid  = ((uint16_t)(pkt[1] & 0x1F) << 8) | pkt[2];
        h.afc  = (pkt[3] >> 4) & 0x03;
        h.cc   = pkt[3] & 0x0F;
        return h;
    }

    // Attempt to find the first sync byte in a file, reading up to max_scan bytes.
    // Returns the offset of the first valid sync byte (verified by checking the
    // next packet also starts with 0x47), or -1 on failure.
    long find_sync(FILE* f, size_t max_scan = 65536)
    {
        uint8_t buf[TS_PACKET_SIZE * 2];
        long start = ftell(f);
        size_t scanned = 0;

        while (scanned < max_scan)
        {
            size_t to_read = std::min(sizeof(buf), max_scan - scanned);
            size_t got = fread(buf, 1, to_read, f);
            if (got < TS_PACKET_SIZE)
                return -1;

            for (size_t i = 0; i + TS_PACKET_SIZE < got; i++)
            {
                if (buf[i] == TS_SYNC_BYTE && buf[i + TS_PACKET_SIZE] == TS_SYNC_BYTE)
                {
                    long offset = start + (long)scanned + (long)i;
                    fseek(f, offset, SEEK_SET);
                    return offset;
                }
            }
            // Overlap to not miss sync bytes near buffer boundary
            scanned += got - TS_PACKET_SIZE;
            fseek(f, start + (long)scanned, SEEK_SET);
        }
        return -1;
    }

    struct input_state
    {
        FILE* f = nullptr;
        std::string filename;
        uint8_t packet[TS_PACKET_SIZE];
        bool eof = false;
        size_t error_count = 0;
        size_t cc_error_count = 0;
        uint8_t last_cc[PID_COUNT];
        bool cc_valid[PID_COUNT];

        input_state()
        {
            memset(last_cc, 0xFF, sizeof(last_cc));
            memset(cc_valid, 0, sizeof(cc_valid));
        }

        ~input_state()
        {
            if (f)
                fclose(f);
        }

        bool read_packet()
        {
            if (eof)
                return false;
            size_t got = fread(packet, 1, TS_PACKET_SIZE, f);
            if (got < TS_PACKET_SIZE)
            {
                eof = true;
                return false;
            }
            return true;
        }

        // Score a packet: higher is better.
        // 0 = unusable (bad sync), 1 = TEI set, 2 = CC error, 3 = clean
        int score_packet() const
        {
            ts_header h = parse_header(packet);
            if (!h.valid)
                return 0;
            if (h.tei)
                return 1;
            // Check continuity counter
            if (h.pid != 0x1FFF && (h.afc & 0x01)) // Has payload
            {
                if (cc_valid[h.pid])
                {
                    uint8_t expected = (last_cc[h.pid] + 1) & 0x0F;
                    if (h.cc != expected && h.cc != last_cc[h.pid]) // Duplicate allowed
                        return 2;
                }
            }
            return 3;
        }

        // Update CC tracking after a packet is selected from this input
        void update_cc()
        {
            ts_header h = parse_header(packet);
            if (!h.valid)
                return;
            if (h.tei)
            {
                error_count++;
                return;
            }
            if (h.pid != 0x1FFF && (h.afc & 0x01))
            {
                if (cc_valid[h.pid])
                {
                    uint8_t expected = (last_cc[h.pid] + 1) & 0x0F;
                    if (h.cc != expected && h.cc != last_cc[h.pid])
                        cc_error_count++;
                }
                last_cc[h.pid] = h.cc;
                cc_valid[h.pid] = true;
            }
        }
    };
}

//***************************************************************************
// Implementation
//***************************************************************************

bool ts_merge::IsMpegTs(const std::string& filename)
{
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
        return false;
    uint8_t buf[TS_PACKET_SIZE + 1];
    bool result = false;
    if (fread(buf, 1, TS_PACKET_SIZE + 1, f) >= TS_PACKET_SIZE + 1)
        result = (buf[0] == TS_SYNC_BYTE && buf[TS_PACKET_SIZE] == TS_SYNC_BYTE);
    fclose(f);
    return result;
}

bool ts_merge::Process(const std::vector<std::string>& inputs,
                       const std::vector<std::string>& outputs,
                       std::ostream* log,
                       uint8_t verbosity)
{
    if (inputs.size() < 2)
    {
        if (log)
            *log << "Error: MPEG-TS merge requires at least 2 input files." << std::endl;
        return false;
    }
    if (outputs.empty())
    {
        if (log)
            *log << "Error: No output file specified for MPEG-TS merge." << std::endl;
        return false;
    }

    // Open inputs
    std::vector<input_state*> inp;
    for (const auto& filename : inputs)
    {
        auto* st = new input_state();
        st->filename = filename;
        st->f = fopen(filename.c_str(), "rb");
        if (!st->f)
        {
            if (log)
                *log << "Error: Cannot open input \"" << filename << "\"." << std::endl;
            delete st;
            for (auto* s : inp) delete s;
            return false;
        }
        // Find sync
        long sync_offset = find_sync(st->f);
        if (sync_offset < 0)
        {
            if (log)
                *log << "Error: No MPEG-TS sync found in \"" << filename << "\"." << std::endl;
            delete st;
            for (auto* s : inp) delete s;
            return false;
        }
        if (sync_offset > 0 && log && verbosity >= 5)
            *log << "Info: Sync found at offset " << sync_offset << " in \"" << filename << "\"." << std::endl;
        inp.push_back(st);
    }

    // Open output
    FILE* out_f = fopen(outputs[0].c_str(), "wb");
    if (!out_f)
    {
        if (log)
            *log << "Error: Cannot open output \"" << outputs[0] << "\"." << std::endl;
        for (auto* s : inp) delete s;
        return false;
    }

    // Additional output copies (if specified)
    std::vector<FILE*> out_copies;
    for (size_t i = 1; i < outputs.size(); i++)
    {
        FILE* fc = fopen(outputs[i].c_str(), "wb");
        if (fc)
            out_copies.push_back(fc);
    }

    // Stats
    stats st;
    st.InputErrorCounts.resize(inp.size(), 0);
    st.InputCcErrors.resize(inp.size(), 0);
    st.InputUsedCounts.resize(inp.size(), 0);

    // Output CC tracking for final stats
    uint8_t out_last_cc[PID_COUNT];
    bool out_cc_valid[PID_COUNT];
    memset(out_last_cc, 0xFF, sizeof(out_last_cc));
    memset(out_cc_valid, 0, sizeof(out_cc_valid));

    if (log && verbosity >= 5)
    {
        *log << "MPEG-TS merge: " << inputs.size() << " inputs -> \"" << outputs[0] << "\"" << std::endl;
    }

    // Main merge loop
    size_t packet_num = 0;
    size_t progress_interval = 10000;
    for (;;)
    {
        // Read one packet from each input
        bool any_alive = false;
        for (auto* s : inp)
        {
            if (s->read_packet())
                any_alive = true;
        }
        if (!any_alive)
            break;

        // Score each input's packet
        int best_score = -1;
        size_t best_idx = 0;
        for (size_t i = 0; i < inp.size(); i++)
        {
            if (inp[i]->eof)
                continue;
            int score = inp[i]->score_packet();
            if (score > best_score)
            {
                best_score = score;
                best_idx = i;
            }
        }

        if (best_score <= 0)
        {
            // All inputs have invalid sync at this position — write first available
            for (size_t i = 0; i < inp.size(); i++)
            {
                if (!inp[i]->eof)
                {
                    best_idx = i;
                    break;
                }
            }
        }

        // Track per-input errors (TEI)
        for (size_t i = 0; i < inp.size(); i++)
        {
            if (inp[i]->eof)
                continue;
            ts_header h = parse_header(inp[i]->packet);
            if (h.valid && h.tei)
                st.InputErrorCounts[i]++;
            // Track CC errors per input
            inp[i]->update_cc();
        }

        // Check if we recovered (best is clean but others had errors)
        bool recovered = false;
        if (best_score >= 2 && inp.size() > 1)
        {
            for (size_t i = 0; i < inp.size(); i++)
            {
                if (i == best_idx || inp[i]->eof)
                    continue;
                int other_score = inp[i]->score_packet();
                if (other_score < best_score)
                {
                    recovered = true;
                    break;
                }
            }
        }
        if (recovered)
            st.RecoveredPackets++;

        // Track output CC
        ts_header out_h = parse_header(inp[best_idx]->packet);
        if (out_h.valid && out_h.tei)
            st.ErrorPackets++;
        if (out_h.valid && out_h.pid != 0x1FFF && (out_h.afc & 0x01))
        {
            if (out_cc_valid[out_h.pid])
            {
                uint8_t expected = (out_last_cc[out_h.pid] + 1) & 0x0F;
                if (out_h.cc != expected && out_h.cc != out_last_cc[out_h.pid])
                    st.CcErrors++;
            }
            out_last_cc[out_h.pid] = out_h.cc;
            out_cc_valid[out_h.pid] = true;
        }

        // Write selected packet
        fwrite(inp[best_idx]->packet, 1, TS_PACKET_SIZE, out_f);
        for (FILE* fc : out_copies)
            fwrite(inp[best_idx]->packet, 1, TS_PACKET_SIZE, fc);

        st.InputUsedCounts[best_idx]++;
        st.TotalPackets++;
        packet_num++;

        // Progress
        if (log && verbosity > 0 && verbosity <= 7 && (packet_num % progress_interval == 0))
        {
            *log << "\rMerging: " << packet_num << " packets processed, "
                 << st.RecoveredPackets << " recovered..." << std::flush;
        }
    }

    // Finalize
    fclose(out_f);
    for (FILE* fc : out_copies)
        fclose(fc);

    // Collect per-input CC error stats
    for (size_t i = 0; i < inp.size(); i++)
        st.InputCcErrors[i] = inp[i]->cc_error_count;

    // Report
    if (log && verbosity > 0)
    {
        if (verbosity <= 7)
            *log << "\r" << std::string(60, ' ') << "\r"; // Clear progress line

        *log << "MPEG-TS Merge Summary" << std::endl;
        *log << "---------------------" << std::endl;
        *log << "Total packets:    " << std::setw(10) << st.TotalPackets << std::endl;

        for (size_t i = 0; i < inp.size(); i++)
        {
            float use_pct = st.TotalPackets ? (float)st.InputUsedCounts[i] / st.TotalPackets * 100.0f : 0;
            *log << "  Input " << i << " (" << inputs[i] << "):" << std::endl;
            *log << "    TEI errors:   " << std::setw(10) << st.InputErrorCounts[i] << std::endl;
            *log << "    CC errors:    " << std::setw(10) << st.InputCcErrors[i] << std::endl;
            *log << "    Packets used: " << std::setw(10) << st.InputUsedCounts[i]
                 << " (" << std::fixed << std::setprecision(2) << use_pct << "%)" << std::endl;
        }

        *log << "Output:" << std::endl;
        *log << "    Recovered:    " << std::setw(10) << st.RecoveredPackets;
        if (st.TotalPackets)
            *log << " (" << std::fixed << std::setprecision(4)
                 << (float)st.RecoveredPackets / st.TotalPackets * 100.0f << "%)";
        *log << std::endl;
        *log << "    TEI remaining:" << std::setw(10) << st.ErrorPackets;
        if (st.TotalPackets)
            *log << " (" << std::fixed << std::setprecision(4)
                 << (float)st.ErrorPackets / st.TotalPackets * 100.0f << "%)";
        *log << std::endl;
        *log << "    CC errors:    " << std::setw(10) << st.CcErrors << std::endl;

        if (st.RecoveredPackets > 0)
            *log << "Result: " << st.RecoveredPackets << " packet(s) recovered from alternate input(s)." << std::endl;
        else
            *log << "Result: No packets needed recovery." << std::endl;
    }

    for (auto* s : inp) delete s;
    return true;
}
