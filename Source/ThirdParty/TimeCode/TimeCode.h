/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <string>
using namespace std;
//---------------------------------------------------------------------------

//***************************************************************************
// Class TimeCode
//***************************************************************************

class TimeCode
{
public:
    //constructor/Destructor
    TimeCode ();
    TimeCode (uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t Frames, uint8_t FramesPerSecond, bool DropFrame, bool MustUseSecondField=false, bool IsSecondField=false);
    TimeCode (int64_t Frames, uint8_t FramesPerSecond, bool DropFrame, bool MustUseSecondField=false, bool IsSecondField_=false);

    //Operators
    TimeCode &operator ++()
    {
        PlusOne();
        return *this;
    }
    TimeCode operator ++(int)
    {
        PlusOne();
        return *this;
    }
    TimeCode& operator +=(int64_t);
    TimeCode& operator --()
    {
        MinusOne();
        return *this;
    }
    TimeCode operator --(int)
    {
        MinusOne();
        return *this;
    }
    bool operator== (const TimeCode &tc) const
    {
        return Hours                ==tc.Hours
            && Minutes              ==tc.Minutes
            && Seconds              ==tc.Seconds
            && Frames               ==tc.Frames
            && FramesPerSecond      ==tc.FramesPerSecond
            && DropFrame            ==tc.DropFrame
            && MustUseSecondField   ==tc.MustUseSecondField
            && IsSecondField        ==tc.IsSecondField;
    }
    bool operator!= (const TimeCode &tc) const
    {
        return !(*this == tc);
    }

    //Helpers
    bool IsValid() const
    {
        return FramesPerSecond && HasValue();
    }
    bool HasValue() const
    {
        return Hours!=(uint8_t)-1;
    }
    void PlusOne();
    void MinusOne();
    bool FromString(const char* Value);
    bool FromString(const string& Value) { return FromString(Value.c_str()); }
    string ToString() const;
    void FromFrames(int64_t Frames);
    int64_t ToFrames() const;
    int64_t ToMilliseconds() const;

public:
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
    uint8_t Frames;
    int32_t MoreSamples;
    int32_t MoreSamples_Frequency;
    bool  FramesPerSecond_Is1001;
    uint8_t FramesPerSecond;
    bool  DropFrame;
    bool  MustUseSecondField;
    bool  IsSecondField;
    bool  IsNegative;
};

TimeCode operator +(const TimeCode&, int64_t);
