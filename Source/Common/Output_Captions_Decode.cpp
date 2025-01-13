/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/Output.h"
#include "Common/Output_Captions_Decode.h"
#include "Common/ProcessFile.h"
#include "ccdecoder_line21.h"
#include "ccdecoder_subrip.h"
#include "ccdecoder_onscreen.h"
#include <algorithm>
#include <fstream>
//---------------------------------------------------------------------------

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
static const char* const Writer_Name = "Captions SCC";

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
static const char Hex2String[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

//---------------------------------------------------------------------------
static const char* CaptionChannels[8] =
{
    ".cc1",
    ".cc2",
    ".t1",
    ".t2",
    ".cc3",
    ".cc4",
    ".t1",
    ".t4",
};

//---------------------------------------------------------------------------
static void InjectBeforeExtension(string& Name, const char* ToInject, size_t Index)
{
    auto DotPos = Name.rfind('.');
    if (DotPos == string::npos)
        DotPos = Name.size();
    Name.insert(DotPos, ToInject + to_string(Index));
}

//---------------------------------------------------------------------------
static void InjectBeforeExtension(string& Name, const char* ToInject)
{
    auto DotPos = Name.rfind('.');
    if (DotPos == string::npos)
        DotPos = Name.size();
    Name.insert(DotPos, ToInject);
}

//---------------------------------------------------------------------------
static void AddWideString(string& Text, const wchar_t* Z)
{
    while (*Z)
    {
        if (*Z < 0x80)
        {
            Text += (char)(*(Z++));
            continue;
        }

        uint32_t wc; // must be unsigned.

        #if defined(_MSC_VER)
        #pragma warning(push)
        #pragma warning(disable:4127)
        #endif //defined(_MSC_VER)
        if (sizeof(wchar_t) == 2)
        #if defined(_MSC_VER)
        #pragma warning(pop)
        #endif //defined(_MSC_VER)
        {
            if (((*Z) & 0xFC00) == 0xD800)
            {
                //UTF-16
                wc = ((((uint16_t)*Z) & 0x3FF) + 0x40) << 10;
                Z++;
                wc |= (((uint16_t)*Z) & 0x3FF);
            }
            else
                wc = (uint16_t)*Z; // avoid a cast problem if wchar_t is signed.
        }
        else
            wc = *Z;

        int count;

        // refer to http://en.wikipedia.org/wiki/UTF-8#Description

        if (wc < 0x80)
            count = 1;
        else if (wc < 0x800)
            count = 2;
        else if (wc < 0x10000)
            count = 3;
        else if (wc < 0x200000)
            count = 4;
        else
            break;  // bad character

        uint64_t utfbuf = 0; // 8 bytes
        char* utf8chars = (char*)&utfbuf;

        switch (count)
        {
        case 4:
            utf8chars[3] = 0x80 | (wc & 0x3f);
            wc = (wc >> 6) | 0x10000;
        case 3:
            utf8chars[2] = 0x80 | (wc & 0x3f);
            wc = (wc >> 6) | 0x800;
        case 2:
            utf8chars[1] = 0x80 | (wc & 0x3f);
            wc = (wc >> 6) | 0xc0;
        case 1:
            utf8chars[0] = (char)wc;
        }

        Text += utf8chars;

        ++Z;
    }
}

//---------------------------------------------------------------------------
struct decoded_data
{
    string Text;
    ofstream* Out = nullptr;

    decoded_data() {};
    ~decoded_data()
    {
        delete Out;
    }

    decoded_data(const decoded_data&) = delete;
    decoded_data(decoded_data&& rhs)
    {
        Text = move(rhs.Text);
        Out = rhs.Out;
        rhs.Out = nullptr; // Older GCC don't have ofstream move() so we use a pointer instead
    }
    decoded_data& operator= (decoded_data&&) = delete;

    return_value Open(const string& OutName, ostream* Err)
    {
        if (Verbosity == 10)
            *Err << "Debug: opening (out, trunc) \"" << OutName << "\"..." << endl;
        Out = new ofstream(OutName, ios_base::trunc);
        if (Verbosity == 10)
            *Err << "Debug: opening (out, trunc) \"" << OutName << "\"... Done." << endl;
        if (!Out->is_open())
            return ReturnValue_ERROR;
        return ReturnValue_OK;
    }
};

//***************************************************************************
// Output
//***************************************************************************

//---------------------------------------------------------------------------
static return_value Output_Captions_Decode(const string& ScreenOutName, const string& SrtOutName, const vector<file::captions_fielddata>& PerFrame_Captions, const vector<size_t>& IgnoredFrames, int Field, ostream* Err)
{
    auto ToReturn = ReturnValue_OK;

    // Open file
    vector<decoded_data> DecodedData[2];

    // File header
    auto handle = ccdecoder_line21_alloc();

    // By Frame - For each line
    for (const auto& Frame : PerFrame_Captions)
    {
        if (find(IgnoredFrames.begin(), IgnoredFrames.end(), Frame.StartFrameNumber) != IgnoredFrames.end())
            continue;

        for (size_t i = 0; i < Frame.Captions.size(); i++)
        {
            const auto& Caption = Frame.Captions[i];
            const auto PTS = Frame.PTS + Frame.DUR * i;
            auto status = ccdecoder_line21_parse(handle, (uint8_t*)&Caption.Data[0], 2, Field ? ccdecoder_fromfield2 : ccdecoder_fromfield1, PTS, PTS, Frame.DUR);
            if (status&ccdecoder_haschanged)
            {
                for (size_t transport_pos = 0; transport_pos < handle->count; transport_pos++)
                    if (handle->transports[transport_pos])
                        for (size_t caption_pos = 0; caption_pos < handle->transports[transport_pos]->count; caption_pos++)
                            if (handle->transports[transport_pos]->captions[caption_pos] && handle->transports[transport_pos]->captions[caption_pos]->status&ccdecoder_haschanged)
                            {
                                // Display instant screen
                                if (!ScreenOutName.empty())
                                {
                                    auto Data = ccdecoder_onscreen(handle->transports[transport_pos]->captions[caption_pos], PTS);
                                    if (Data)
                                    {
                                        if (caption_pos >= DecodedData[0].size())
                                            DecodedData[0].resize(caption_pos + 1);
                                        AddWideString(DecodedData[0][caption_pos].Text, Data);
                                        free(Data);
                                    }
                                }

                                // Creating the SubRip handle if it is not yet created, then display
                                if (!SrtOutName.empty())
                                {
                                    if (!handle->transports[transport_pos]->captions[caption_pos]->user)
                                        handle->transports[transport_pos]->captions[caption_pos]->user = ccdecoder_subrip_alloc();
                                    auto Data = ccdecoder_subrip_parse((ccdecoder_subrip_handle*)handle->transports[transport_pos]->captions[caption_pos]->user, handle->transports[transport_pos]->captions[caption_pos], PTS);
                                    if (Data)
                                    {
                                        if (caption_pos >= DecodedData[1].size())
                                            DecodedData[1].resize(caption_pos + 1);
                                        AddWideString(DecodedData[1][caption_pos].Text, Data);
                                        free(Data);
                                    }
                                }
                            }
            }
        }

        // Flush
        for (size_t transport_pos = 0; transport_pos < handle->count; transport_pos++)
            if (handle->transports[transport_pos])
                for (size_t caption_pos = 0; caption_pos < handle->transports[transport_pos]->count; caption_pos++)
                    if (handle->transports[transport_pos]->captions[caption_pos] && handle->transports[transport_pos]->captions[caption_pos]->status&ccdecoder_haschanged)
                        if (handle->transports[transport_pos]->captions[caption_pos]->user)
                        {
                            auto Data = ccdecoder_subrip_parse((ccdecoder_subrip_handle*)handle->transports[transport_pos]->captions[caption_pos]->user, handle->transports[transport_pos]->captions[caption_pos], Frame.PTS + (Frame.Captions.size() + 1) * Frame.DUR);
                            if (Data)
                            {
                                if (caption_pos >= DecodedData[1].size())
                                    DecodedData[1].resize(caption_pos + 1);
                                AddWideString(DecodedData[1][caption_pos].Text, Data);
                                free(Data);
                            }
                        }

        // Write content to output
        for (int i = 0; i < 2; i++)
            for (size_t j = 0; j < DecodedData[i].size(); j++)
            {
                auto& Caption = DecodedData[i][j];
                if (!Caption.Out)
                {
                    string OutNameWithChannel(i ? SrtOutName : ScreenOutName);
                    InjectBeforeExtension(OutNameWithChannel, j < 8 ? CaptionChannels[j] : to_string(j).c_str());
                    if (Caption.Open(OutNameWithChannel, Err))
                    {
                        if (Err)
                            *Err << "Error: can not open " << OutNameWithChannel << " for writing.\n";
                        ToReturn = ReturnValue_ERROR;
                    }
                }
                if (Caption.Out && Caption.Out->is_open() && !WriteIfBig(*Caption.Out, Caption.Text, Err, Writer_Name))
                    ToReturn = ReturnValue_ERROR;
            }
    }

    // Write content to output
    for (int i = 0; i < 2; i++)
        for (size_t j = 0; j < DecodedData[i].size(); j++)
        {
            auto& Caption = DecodedData[i][j];
            if (Caption.Out && Caption.Out->is_open() && !Write(*Caption.Out, Caption.Text, Err, Writer_Name))
                ToReturn = ReturnValue_ERROR;
        }

    return ToReturn;
}

//---------------------------------------------------------------------------
return_value Output_Captions_Caption(const Core::OutFile& ScreenOut, const Core::OutFile& SrtOut, const TimeCode* OffsetTimeCode, std::vector<file*>& PerFile, ostream* Err)
{
    auto ToReturn = ReturnValue_OK;

    bool OutputFrames_Speed = true;
    bool OutputFrames_Concealed = true;

    auto It = find(Merge_OutputFileNames.begin(), Merge_OutputFileNames.end(), ScreenOut.Merge_OutputFileName.size() ? ScreenOut.Merge_OutputFileName : SrtOut.Merge_OutputFileName);
    if (It != Merge_OutputFileNames.end())
    {
        size_t Pos = It - Merge_OutputFileNames.begin();
        OutputFrames_Speed = OutputFrames_Speeds[Pos];
        OutputFrames_Concealed = OutputFrames_Concealeds[Pos];
    }

    for (const auto& File : PerFile)
    {
        if (File->PerFrame_Captions_PerSeq_PerField.empty())
            continue; // Show the file only if there is some captions content

        // filter
        vector<size_t> IgnoredFrames;
        if (!OutputFrames_Speed || !OutputFrames_Concealed)
        {
            for (auto Frame = File->PerFrame.begin(); Frame < File->PerFrame.end(); ++Frame)
            {
                coherency_flags Coherency(*Frame);
                if (!OutputFrames_Concealed && Coherency.full_conceal())
                {
                    IgnoredFrames.push_back(Frame - File->PerFrame.begin());
                    continue;
                }

                if (!OutputFrames_Speed && !GetDvSpeedIsNormalPlayback(GetDvSpeed(**Frame)))
                {
                    IgnoredFrames.push_back(Frame - File->PerFrame.begin());
                    continue;
                }
            }
        }

        // Per Dseq
        for (size_t i = 0; i < File->PerFrame_Captions_PerSeq_PerField.size(); i++) // Per Dseq
        {
            for (int j = 0; j < 2; j++) // Per field
            {
                string ScreenOutNameWithDseq(ScreenOut.Name);
                if (!ScreenOutNameWithDseq.empty() && File->PerFrame_Captions_PerSeq_PerField.size() > 1)
                    InjectBeforeExtension(ScreenOutNameWithDseq, ".dseq", i);
                string SrtOutNameWithDseq(SrtOut.Name);
                if (!SrtOutNameWithDseq.empty() && File->PerFrame_Captions_PerSeq_PerField.size() > 1)
                    InjectBeforeExtension(SrtOutNameWithDseq, ".dseq", i);

                if (!File->PerFrame_Captions_PerSeq_PerField[i].FieldData[j].empty() && !Output_Captions_Decode(ScreenOutNameWithDseq, SrtOutNameWithDseq, File->PerFrame_Captions_PerSeq_PerField[i].FieldData[j], IgnoredFrames, j, Err))
                    ToReturn = ReturnValue_ERROR;
            }
        }
    }

    return ToReturn;
}
