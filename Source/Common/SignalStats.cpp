/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "Common/SignalStats.h"
#include "Common/ProcessFileWrapper.h"

#include <ZenLib/Utils.h>

#include <cstring>
#include <cmath>
#include <iostream>


using namespace std;
//---------------------------------------------------------------------------

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
#define BITDEPTH_8_DEPTH   8
#define BITDEPTH_8_OFFSET  (1 << (BITDEPTH_8_DEPTH - 1))
#define BITDEPTH_10_DEPTH  10
#define BITDEPTH_10_OFFSET (1 << (BITDEPTH_10_DEPTH - 1))

//---------------------------------------------------------------------------
void ComputeSatUYVY422(const decklink_frame* Frame, size_t* Sats)
{
    size_t LineSize = Frame->Width * 16 / 8;
    if (Frame->Video_Buffer_Size < Frame->Height * LineSize)
        return;

    const uint8_t* Buffer = (const uint8_t*)Frame->Video_Buffer;
    for (size_t Height = 0; Height < Frame->Height; Height++)
    {
        for (size_t Width = 0; Width < Frame->Width; Width += 2)
        {
            size_t Buffer_Offset = (Height * LineSize) + (Width * 16 / 8);
            int8_t U, V;

            U = Buffer[Buffer_Offset + 2] - BITDEPTH_8_OFFSET;
            V = Buffer[Buffer_Offset]     - BITDEPTH_8_OFFSET;
            Sats[(uint8_t)sqrt(U * U + V * V)]++;
        }
    }
}

//---------------------------------------------------------------------------
void ComputeSatV210(const decklink_frame* Frame, size_t* Sats)
{
    //TODO: Indianness
    size_t LineSize = (((Frame->Width + 47) / 48) * 128) / sizeof(uint32_t);
    if (Frame->Video_Buffer_Size < Frame->Height * (LineSize / 6 * 4) * 4)
        return;

    const uint32_t* Buffer = (const uint32_t*)Frame->Video_Buffer;
    for (size_t Height = 0; Height < Frame->Height; Height++)
    {
        for (size_t Width = 0; Width < Frame->Width; Width += 6)
        {
            size_t Buffer_Offset = Height * LineSize + (Width / 6 * 4);
            int32_t U, V;

            U = (((Buffer[Buffer_Offset]          ) & 0x3FF) - BITDEPTH_10_OFFSET);
            V = (((Buffer[Buffer_Offset]     >> 20) & 0x3FF) - BITDEPTH_10_OFFSET);
            Sats[(uint16_t)sqrt(U * U + V * V)]++;

            U = (((Buffer[Buffer_Offset + 1] >> 10) & 0x3FF) - BITDEPTH_10_OFFSET);
            V = (((Buffer[Buffer_Offset + 2]      ) & 0x3FF) - BITDEPTH_10_OFFSET);
            Sats[(uint16_t)sqrt(U * U + V * V)]++;

            U = (((Buffer[Buffer_Offset + 2] >> 20) & 0x3FF) - BITDEPTH_10_OFFSET);
            V = (((Buffer[Buffer_Offset + 3] >> 10) & 0x3FF) - BITDEPTH_10_OFFSET);
            Sats[(uint16_t)sqrt(U * U + V * V)]++;
        }
    }
}

//***************************************************************************
// ComputeStats
//***************************************************************************

//---------------------------------------------------------------------------
SignalStats::Stats SignalStats::ComputeStats(const decklink_frame* Frame)
{
    Stats Result;

    if (!Frame)
        return Stats();

    size_t Sats_Count = 0, Sats_Size = 0, *Sats = nullptr;
    switch (Frame->Pixel_Format)
    {
    case 1://UYVY
    {
        Sats_Count = (Frame->Width / 2) * Frame->Height;
        Sats_Size = 1 << BITDEPTH_8_DEPTH;
        Sats = new size_t[Sats_Size];
        memset(Sats, 0, Sats_Size * sizeof(size_t));

        ComputeSatUYVY422(Frame, Sats);
    }
    break;
    case 2: //v210
    {
        Sats_Count = (Frame->Width / 2) * Frame->Height;
        Sats_Size = 1 << BITDEPTH_10_DEPTH;
        Sats = new size_t[Sats_Size];
        memset (Sats, 0, Sats_Size * sizeof(size_t));

        ComputeSatV210(Frame, Sats);
    }
    break;
    default:;
    }

    if (Sats)
    {
        int32_t Hi = -1;
        int32_t Max = -1;
        int32_t Count = 0;
        int64_t Total = 0;
        int32_t Sats_Count_90Percentile = lrint(Sats_Count * 90 / 100.0f);
        for (uint16_t Sat = 0; Sat < Sats_Size; Sat++)
        {
            if (Sats[Sat] > 0)
            {
                Count += Sats[Sat];
                Total += Sat * Sats[Sat];
                if (Hi < 0 && Count >= Sats_Count_90Percentile)
                    Hi = Sat;

                Max = Sat;
            }
        }
        Result.SatAvg = 1.0f * Total / Sats_Count;
        Result.SatHi = Hi;
        Result.SatMax = (uint16_t)Max;

        delete[] Sats;
    }

    return Result;
}
