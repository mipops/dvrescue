/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/Core.h"
#include "Common/ProcessFile.h"
#include "Common/XmlOutput.h"
#include "ZenLib/Ztring.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Core::Core()
{
    MediaInfo::Option_Static(__T("ParseSpeed"), __T("1.000"));
}

Core::~Core()
{
    PerFile_Clear();
}

//***************************************************************************
// Process
//***************************************************************************

//---------------------------------------------------------------------------
void Core::Process()
{
    PerFile_Clear();
    
    PerFile.reserve(Inputs.size());
    for (const auto& Input : Inputs)
        PerFile.push_back(new file(Input));
}

//---------------------------------------------------------------------------
float Core::State ()
{
    size_t Total = 0;
    for (const auto& File : PerFile)
        Total += File->MI.State_Get();
    return (((float)Total)/PerFile.size()/10000);
}

//***************************************************************************
// Output
//***************************************************************************

//---------------------------------------------------------------------------
string Core::OutputXml()
{
    return ::OutputXml(PerFile);
}

//***************************************************************************
// PerFile
//***************************************************************************

//---------------------------------------------------------------------------
void Core::PerFile_Clear()
{
    for (const auto& File : PerFile)
        delete File;
    PerFile.clear();
}
