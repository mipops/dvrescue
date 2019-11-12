/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a Unlicense license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/XmlOutput.h"
#include "Common/ProcessFile.h"
#include "ZenLib/Ztring.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

//***************************************************************************
// Sizes
//***************************************************************************

const auto Sta_Bits = 4;
const auto Dseq_Bits = 4;
const auto Sta_BitPos = 0;
const auto Dseq_BitPos = Sta_BitPos + Sta_Bits;
const auto Sta_Size = 1 << Sta_Bits;
const auto Dseq_Size = 1 << Dseq_Bits;
const auto Sta_Step = 1;
const auto Dseq_Step = Sta_Size * Sta_Step;
const auto Dseq_Max = Dseq_Size * Dseq_Step;

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
string TimeCode2String(int Seconds, bool DropFrame, int Frames)
{
    string Value("00:00:00:00");
    Value[0] += Seconds / 36000; Seconds %= 36000;
    Value[1] += Seconds / 3600; Seconds %= 3600;
    Value[3] += Seconds / 600; Seconds %= 600;
    Value[4] += Seconds / 60; Seconds %= 60;
    Value[6] += Seconds / 10; Seconds %= 10;
    Value[7] += Seconds;
    if (Frames < 100)
    {
        if (DropFrame)
            Value[8] = ';';
        Value[9] += Frames / 10;
        Value[10] += Frames % 10;
    }
    else
        Value.resize(8);

    return Value;
}

//---------------------------------------------------------------------------
string to_timestamp(double Seconds_Float)
{
    if (Seconds_Float >= 360000)
        return string(); // Not supported
    string Value("00:00:00.000");
    auto Seconds = int(Seconds_Float);
    Seconds_Float -= Seconds;
    Seconds_Float *= 1000;
    auto MilliSeconds = int(Seconds_Float);
    Seconds_Float -= MilliSeconds;
    if (Seconds_Float >= 0.5)
        MilliSeconds++;
    Value[0] += Seconds / 36000; Seconds %= 36000;
    Value[1] += Seconds / 3600; Seconds %= 3600;
    Value[3] += Seconds / 600; Seconds %= 600;
    Value[4] += Seconds / 60; Seconds %= 60;
    Value[6] += Seconds / 10; Seconds %= 10;
    Value[7] += Seconds;
    Value[9] += MilliSeconds / 100; MilliSeconds %= 100;
    Value[10] += MilliSeconds / 10; MilliSeconds %= 10;
    Value[11] += MilliSeconds;

    return Value;
}

//---------------------------------------------------------------------------
string Date2String(int Years, int Months, int Days)
{
    string Value("2000-00-00");
    if (Years >= 70) // Arbitrary decided
    {
        Value[0] = '1';
        Value[1] = '9';
    }
    Value[2] += Years / 10;
    Value[3] += Years % 10;
    Value[5] += Months / 10;
    Value[6] += Months % 10;
    Value[8] += Days / 10;
    Value[9] += Days % 10;

    return Value;
}

//---------------------------------------------------------------------------
char to_hex4(int Value)
{
    if (Value >= 16)
        return 'X';
    if (Value >= 10)
        return 'A' - 10 + Value;
    return '0' + Value;
}

void Xml_Dseq_Begin(string& Text, size_t o, int Dseq)
{
    Text.append(o, '\t');
    Text += "<dseq n=\"";
    Text += to_string(Dseq >> Dseq_BitPos);
    Text += "\">\n";
}

void Xml_Dseq_End(string& Text, size_t o)
{
    Text.append(o, '\t');
    Text += "</dseq>\n";
}

void Xml_Sta_Element(string& Text, size_t o, int Sta, size_t n)
{
    if (!n)
        return;

    Text.append(o, '\t');
    Text += "<sta t=\"";
    Text += to_string(Sta);
    Text += "\" n=\"";
    Text += to_string(n);
    Text += "\"/>\n";
}

void Xml_Sta_Elements(string& Text, size_t o, const size_t* const Stas)
{
    for (auto Sta = 0; Sta < Sta_Size; Sta++)
    {
        auto n = Stas[Sta];
        Xml_Sta_Element(Text, o, Sta, n);
    }
}

//***************************************************************************
// Output
//***************************************************************************

//---------------------------------------------------------------------------
string OutputXml(std::vector<file*>& PerFile)
{
    string Text;

    // XML header
    Text += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<dvrescue xmlns=\"https://mediaarea.net/dvrescue\" version=\"1.0\">\n"
        "\t<creator>\n"
        "\t\t<program>dvrescue</program>\n"
        "\t\t<version>" Program_Version "</version>\n"
        "\t</creator>\n";

    // XML footer
    Text += "</dvrescue>\n";

    return Text;
}
