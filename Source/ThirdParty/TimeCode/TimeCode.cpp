/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "TimeCode.h"
#include <cmath>
#include <sstream>
//---------------------------------------------------------------------------

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
TimeCode::TimeCode ()
:   Hours((uint8_t)-1),
    Minutes((uint8_t)-1),
    Seconds((uint8_t)-1),
    Frames((uint8_t)-1),
    MoreSamples(0),
    MoreSamples_Frequency(0),
    FramesPerSecond_Is1001(false),
    FramesPerSecond(0),
    DropFrame(false),
    MustUseSecondField(false),
    IsSecondField(false),
    IsNegative(false)
{
}

//---------------------------------------------------------------------------
TimeCode::TimeCode (uint8_t Hours_, uint8_t Minutes_, uint8_t Seconds_, uint8_t Frames_, uint8_t FramesPerSecond_, bool DropFrame_, bool MustUseSecondField_, bool IsSecondField_)
:   Hours(Hours_),
    Minutes(Minutes_),
    Seconds(Seconds_),
    Frames(Frames_),
    MoreSamples(0),
    MoreSamples_Frequency(0),
    FramesPerSecond_Is1001(false),
    FramesPerSecond(FramesPerSecond_),
    DropFrame(DropFrame_),
    MustUseSecondField(MustUseSecondField_),
    IsSecondField(IsSecondField_),
    IsNegative(false)
{
}

//---------------------------------------------------------------------------
TimeCode::TimeCode (int64_t Frames_, uint8_t FramesPerSecond_, bool DropFrame_, bool MustUseSecondField_, bool IsSecondField_)
:   MoreSamples(0),
    MoreSamples_Frequency(0),
    FramesPerSecond_Is1001(false),
    FramesPerSecond(FramesPerSecond_),
    DropFrame(DropFrame_),
    MustUseSecondField(MustUseSecondField_),
    IsSecondField(IsSecondField_)
{
    if (!FramesPerSecond_)
    {
        Frames  = 0;
        Seconds = 0;
        Minutes = 0;
        Hours   = 0;
        IsNegative = true; //Forcing a weird display
        return;
    }

    FromFrames(Frames_);
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void TimeCode::PlusOne()
{
    //TODO: negative values

    if (FramesPerSecond==0)
        return;
    if (MustUseSecondField)
    {
        if (IsSecondField)
        {
            Frames++;
            IsSecondField=false;
        }
        else
            IsSecondField=true;
    }
    else
        Frames++;
    if (Frames>=FramesPerSecond)
    {
        Seconds++;
        Frames=0;
        if (Seconds>=60)
        {
            Seconds=0;
            Minutes++;

            if (DropFrame && Minutes%10)
                Frames=2; //frames 0 and 1 are dropped for every minutes except 00 10 20 30 40 50

            if (Minutes>=60)
            {
                Minutes=0;
                Hours++;
                if (Hours>=24)
                {
                    Hours=0;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
void TimeCode::MinusOne()
{
    //TODO: negative values

    if (FramesPerSecond==0)
        return;
    if (MustUseSecondField && IsSecondField)
        IsSecondField=false;
    else
    {
        if (Frames==0 || (DropFrame && Minutes%10 && Frames<=2))
        {
            Frames=FramesPerSecond;
            if (Seconds==0)
            {
                Seconds=60;
                if (Minutes==0)
                {
                    Minutes=60;
                    if (Hours==0)
                        Hours=24;
                    Hours--;
                }
                Minutes--;
            }
            Seconds--;
        }
        Frames--;

        if (MustUseSecondField)
            IsSecondField=true;
    }
}

//---------------------------------------------------------------------------
TimeCode& TimeCode::operator +=(int64_t Value)
{
    int64_t Frames_=ToFrames();
    Frames_+=Value;
    FromFrames(Frames_);
    return *this;
}

//---------------------------------------------------------------------------
bool TimeCode::FromString(const char* Value)
{
    //TODO: negative values

    if (Value[ 0] < '0' || Value[ 0] > '9'
     || Value[ 1] < '0' || Value[ 1] > '9'
     || Value[ 2] != ':'
     || Value[ 3] < '0' || Value[ 3] > '9'
     || Value[ 4] < '0' || Value[ 4] > '9'
     || Value[ 5] != ':'
     || Value[ 6] < '0' || Value[ 6] > '9'
     || Value[ 7] < '0' || Value[ 7] > '9'
     || (Value[ 8] != ':' && Value[8] != ';')
     || Value[ 9] < '0' || Value[ 9] > '9'
     || Value[10] < '0' || Value[10] > '9'
     || Value[11])
        return true;
    
    Hours       = (Value[ 0]-'0') * 10 + Value[ 1]-'0';
    Minutes     = (Value[ 3]-'0') * 10 + Value[ 4]-'0';
    Seconds     = (Value[ 6]-'0') * 10 + Value[ 7]-'0';
    DropFrame   =  Value[ 8] == ';';
    Frames      = (Value[ 9]-'0') * 10 + Value[10]-'0';
    return false;
}

//---------------------------------------------------------------------------
string TimeCode::ToString() const
{
    if (!IsValid())
        return string();

    string TC;
    if (IsNegative)
        TC+='-';
    TC+=('0'+Hours/10);
    TC+=('0'+Hours%10);
    TC+=':';
    TC+=('0'+Minutes/10);
    TC+=('0'+Minutes%10);
    TC+=':';
    TC+=('0'+Seconds/10);
    TC+=('0'+Seconds%10);
    TC+=DropFrame?';':':';
    TC+=('0'+(Frames*(MustUseSecondField?2:1)+(IsSecondField?1:0))/10);
    TC+=('0'+(Frames*(MustUseSecondField?2:1)+(IsSecondField?1:0))%10);
    if (MoreSamples && MoreSamples_Frequency)
    {
        int32_t MoreSamplesTemp=MoreSamples;
        if (MoreSamplesTemp>0)
            TC+='+';
        else
        {
            TC+='-';
            MoreSamplesTemp=-MoreSamplesTemp;
        }
        stringstream s;
        s<<MoreSamplesTemp;
        TC+=s.str();
        TC+='/';
        s.str(string());
        s<<MoreSamples_Frequency;
        TC+=s.str();
    }

    return TC;
}

//---------------------------------------------------------------------------
void TimeCode::FromFrames(int64_t Frames_)
{
    if (!Frames_)
    {
        Frames = 0;
        Seconds = 0;
        Minutes = 0;
        Hours = 0;
        return;
    }

    if (!FramesPerSecond)
        return;

    if (Frames_<0)
    {
        IsNegative=true;
        Frames_=-Frames_;
    }
    else
        IsNegative=false;

    uint8_t Dropped=0;
    if (DropFrame)
    {
        Dropped=2;
        if (FramesPerSecond>30)
            Dropped+=2;
        if (FramesPerSecond>60)
            Dropped+=2;
        if (FramesPerSecond>90)
            Dropped+=2;
        if (FramesPerSecond>120)
            Dropped+=2;
    }

    uint64_t Minutes_Tens=Frames_/(600*FramesPerSecond-Dropped*9); //Count of 10 minutes
    uint64_t Minutes_Units=(Frames_-Minutes_Tens*(600*FramesPerSecond-Dropped*9))/(60*FramesPerSecond-Dropped);

    Frames_+=9*Dropped*Minutes_Tens+Dropped*Minutes_Units;
    if (Minutes_Units && ((Frames_/FramesPerSecond)%60)==0 && (Frames_%FramesPerSecond)<Dropped) // If Minutes_Tens is not 0 (drop) but count of remaining seconds is 0 and count of remaining frames is less than 2, 1 additional drop was actually counted, removing it
        Frames_-=Dropped;

    Frames  =    Frames_ % FramesPerSecond;
    Seconds =   (Frames_ / FramesPerSecond) % 60;
    Minutes =  ((Frames_ / FramesPerSecond) / 60) % 60;
    Hours   = (((Frames_ / FramesPerSecond) / 60) / 60) % 24;
}

//---------------------------------------------------------------------------
int64_t TimeCode::ToFrames() const
{
    if (!FramesPerSecond)
        return 0;

    int64_t TC=(int64_t(Hours)     *3600
             + int64_t(Minutes)   *  60
             + int64_t(Seconds)        )*int64_t(FramesPerSecond)
             + int64_t(Frames);

    if (DropFrame)
    {
        TC-= int64_t(Hours)      *108
          + (int64_t(Minutes)/10)*18
          + (int64_t(Minutes)%10)*2;
    }

    TC*=(MustUseSecondField?2:1);
    TC+=(IsSecondField?1:0);

    return IsNegative?-TC:TC;
}

//---------------------------------------------------------------------------
int64_t TimeCode::ToMilliseconds() const
{
    if (!FramesPerSecond)
        return 0;

    int64_t MS=lround(ToFrames()*1000*(DropFrame?1.001:1.000)/(FramesPerSecond*(MustUseSecondField?2:1)));

    return IsNegative?-MS:MS;
}

//---------------------------------------------------------------------------
TimeCode operator +(const TimeCode& a, int64_t b)
{
    TimeCode c(a);
    c+=b;
    return c;
}
