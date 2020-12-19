/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/Output.h"
#include <cmath>
//---------------------------------------------------------------------------

//***************************************************************************
// Arrays
//***************************************************************************

extern const char* const chroma_subsampling[chroma_subsampling_size] =
{
    "4:1:1",
    "4:2:0",
    "4:2:2",
};

//***************************************************************************
// Formating helpers
//***************************************************************************

//---------------------------------------------------------------------------
void timecode_to_string(string& Data, int Seconds, bool DropFrame, int Frames)
{
    if (Seconds >= 360000)
        return;
    Data.append("00:00:00:00", 11);
    auto Value = Data.end() - 11;
    Value[0] += Seconds / 36000; Seconds %= 36000;
    Value[1] += Seconds / 3600; Seconds %= 3600;
    Value[3] += Seconds / 600; Seconds %= 600;
    Value[4] += Seconds / 60; Seconds %= 60;
    Value[6] += Seconds / 10; Seconds %= 10;
    Value[7] += Seconds;
    if (Frames < 0x3F) // 6-bit, sometimes set to 0x3F when unknown
    {
        if (DropFrame)
            Value[8] = ';';
        Value[9] += Frames / 10;
        Value[10] += Frames % 10;
    }
    else
        Data.resize(Data.size() - 3);
}

//---------------------------------------------------------------------------
void seconds_to_timestamp(string& Data, double Seconds_Float, int CountAfterComma, bool trunc)
{
    if (Seconds_Float >= 360000 || CountAfterComma < 0 || CountAfterComma > 9)
        return; // Not supported
    int Powered;
    if (CountAfterComma)
    {
        auto Powered_Float = pow(10, CountAfterComma);
        Powered = int(Powered_Float);
        Seconds_Float *= Powered_Float;
    }
    auto Seconds_Multiplied = (unsigned long long)(trunc?floor(Seconds_Float):lround(Seconds_Float));
    auto Seconds = Seconds_Multiplied;
    int AfterComma;
    if (CountAfterComma)
    {
        Seconds /= Powered;
        AfterComma = Seconds_Multiplied % Powered;
    }
    auto HundredsOfHours = Seconds / 360000;
    if (HundredsOfHours)
    {
        auto HundredsOfHoursString = to_string(HundredsOfHours);
        Data.append(HundredsOfHoursString);
        Seconds %= 360000;
    }
    auto S = int(Seconds); // It is guaranted to be less than 360000 so fits in an int
    Data.append("00:00:00");
    auto Value = &Data.back() - 7;
    Value[0] += S / 36000; S %= 36000;
    Value[1] += S / 3600; S %= 3600;
    Value[3] += S / 600; S %= 600;
    Value[4] += S / 60; S %= 60;
    Value[6] += S / 10; S %= 10;
    Value[7] += S;
    if (CountAfterComma)
    {
        auto AfterCommaString = to_string(AfterComma);
        AfterCommaString.insert(0, CountAfterComma - AfterCommaString.size(), '0');
        Data += '.';
        Data += AfterCommaString;
    }
}

//---------------------------------------------------------------------------
void date_to_string(string& Data, int Years, int Months, int Days)
{
    Data.append("2000-00-00");
    auto Value = &Data.back() - 9;
    if (Years >= 70) // Arbitrary decided
    {
        Value[0] = '1';
        Value[1] = '9';
    }
    Value[2] += Years / 10;
    Value[3] += Years % 10;
    if (Months < 100)
    {
        Value[5] += Months / 10;
        Value[6] += Months % 10;
        if (Days < 100)
        {
            Value[8] += Days / 10;
            Value[9] += Days % 10;
        }
        else
            Data.resize(Data.size() - 3);
    }
    else
        Data.resize(Data.size() - 6);
}

//---------------------------------------------------------------------------
char uint4_to_hex4(int Value)
{
    if (((unsigned int)Value) >= 16)
        return 'X';
    if (Value >= 10)
        return 'A' - 10 + Value;
    return '0' + Value;
}

//***************************************************************************
// Status of a frame
//***************************************************************************

bool Frame_HasErrors(const MediaInfo_Event_DvDif_Analysis_Frame_1& Frame)
{
    return Frame.Video_STA_Errors || Frame.Audio_Data_Errors
        || ((Frame.TimeCode >> 30) & 0x1) || ((Frame.RecordedDateTime1 >> 30) & 0x1) || (Frame.Arb & (1 << 6)); // Non consecutive
}

computed_errors::computed_errors()
{
    memset(this, 0, sizeof(*this));
}

bool computed_errors::Compute(const MediaInfo_Event_DvDif_Analysis_Frame_1& Frame)
{
    bool HasErrors = false;
    for (int Dseq = 0; Dseq < Dseq_Size; Dseq++)
        HasErrors |= Compute(Frame, Dseq);
    return HasErrors;
}

bool computed_errors::Compute(const MediaInfo_Event_DvDif_Analysis_Frame_1& Frame, int Dseq)
{
    memset(&PerDseq, 0, sizeof(PerDseq));
    bool HasErrors = false;
    for (auto Sta = 0; Sta < Sta_Size; Sta++)
    {
        if (Frame.Video_STA_Errors)
        {
            auto DseqSta = (Dseq << Sta_Bits) | Sta;
            const auto n = Frame.Video_STA_Errors[DseqSta];
            if (n)
            {
                if (!HasErrors)
                    HasErrors = true;
                PerDseq.Video_Sta_TotalPerSta[Sta] += n;
                Video_Sta_TotalPerSta[Sta] += n;
                if (!(Dseq % 2))
                {
                    PerDseq.Video_Sta_EvenTotalPerSta[Sta] += n;
                    Video_Sta_EvenTotalPerSta[Sta] += n;
                }
            }
        }
    }
    if (Frame.Audio_Data_Errors)
    {
        const auto n = Frame.Audio_Data_Errors[Dseq];
        if (n)
        {
            if (!HasErrors)
                HasErrors = true;
            PerDseq.Audio_Data_Total += n;
            Audio_Data_Total += n;
            if (!(Dseq % 2))
            {
                PerDseq.Audio_Data_EvenTotal += n;
                Audio_Data_EvenTotal += n;
            }
        }
    }

    return HasErrors;
}

//***************************************************************************
// Writing
//***************************************************************************

return_value Write(ostream& Out, string& ToWrite, ostream* Err, const char* const OutName, size_t ToWrite_Size)
{
    if (ToWrite.empty())
        return ReturnValue_OK;

    if (!ToWrite_Size || ToWrite_Size > ToWrite.size())
        ToWrite_Size = ToWrite.size();
    Out.write(ToWrite.c_str(), ToWrite_Size);
    ToWrite.erase(0, ToWrite_Size);
    if (!Out.good())
    {
        if (Err)
        {
            *Err << "Error: can not write to ";
            if (OutName)
                *Err << OutName << ' ';
            *Err << "output.\n";
        }
        return ReturnValue_ERROR;
    }

    return ReturnValue_OK;
}