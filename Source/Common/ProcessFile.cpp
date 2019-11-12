/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a Unlicense license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/ProcessFile.h"
#include "ZenLib/Ztring.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

//***************************************************************************
// Callback
//***************************************************************************

void __stdcall Event_CallBackFunction(unsigned char* Data_Content, size_t Data_Size, void* UserHandler_Void)
{
    //Retrieving UserHandler
    file*                               UserHandler = (file*)UserHandler_Void;
    struct MediaInfo_Event_Generic*     Event_Generic = (struct MediaInfo_Event_Generic*) Data_Content;
    unsigned char                       ParserID;
    unsigned short                      EventID;
    unsigned char                       EventVersion;

    //Integrity test
    if (Data_Size < 4)
        return; //There is a problem

    //Retrieving EventID
    ParserID = (unsigned char)((Event_Generic->EventCode & 0xFF000000) >> 24);
    EventID = (unsigned short)((Event_Generic->EventCode & 0x00FFFF00) >> 8);
    EventVersion = (unsigned char)(Event_Generic->EventCode & 0x000000FF);
    switch (ParserID)
    {
    case MediaInfo_Parser_DvDif:
        switch (EventID)
        {
        }
        break;
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
file::file(const String& FileName)
{
    MI.Option(__T("File_Event_CallBackFunction"), __T("CallBack=memory://") + Ztring::ToZtring((size_t)&Event_CallBackFunction) + __T(";UserHandler=memory://") + Ztring::ToZtring((size_t)this));
    MI.Option(__T("File_DvDif_Analysis"), __T("1"));
    MI.Open(FileName);

    // Filing some info
    FrameRate = Ztring(MI.Get(Stream_Video, 0, __T("FrameRate_Original"))).To_float64();
    if (!FrameRate)
        FrameRate = Ztring(MI.Get(Stream_Video, 0, __T("FrameRate"))).To_float64();
    if (!FrameRate || (FrameRate >= 29.97 && FrameRate <= 29.98))
        FrameRate = double(30 / 1.001); // Default if no frame rate available, or better rounding
}

//---------------------------------------------------------------------------
file::~file()
{
}
