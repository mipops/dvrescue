/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#pragma once
#include "Common/Config.h"
#ifdef MEDIAINFO_DLL
    #include "MediaInfoDLL/MediaInfoDLL.h"
    #define MediaInfoNameSpace MediaInfoDLL
#elif defined MEDIAINFO_STATIC
    #include "MediaInfoDLL/MediaInfoDLL_Static.h"
    #define MediaInfoNameSpace MediaInfoDLL
#else
    #include "MediaInfo/MediaInfoList.h"
    #define MediaInfoNameSpace MediaInfoLib
#endif
#include "MediaInfo/MediaInfo_Events.h"
#include <vector>
using namespace MediaInfoNameSpace;
using namespace std;
//---------------------------------------------------------------------------

//***************************************************************************
// Class file
//***************************************************************************

class file
{
public:
    MediaInfo MI;
    std::vector<MediaInfo_Event_DvDif_Change_0*> PerChange;
    std::vector<MediaInfo_Event_DvDif_Analysis_Frame_1*> PerFrame;
    size_t FrameNumber;
    double FrameRate;

    file(const String& FileName);
    ~file();

    void AddChange(const MediaInfo_Event_DvDif_Change_0* FrameData);
    void AddFrame(const MediaInfo_Event_DvDif_Analysis_Frame_1* FrameData);
};
