/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "CLI/CLI_Help.h"
#include "Common/ProcessFile.h"
#include <ostream>
using namespace std;
//---------------------------------------------------------------------------

//***************************************************************************
// Help
//***************************************************************************

//---------------------------------------------------------------------------
return_value Help(ostream& Out, const char* Name, bool Full)
{
    Out <<
    "Usage: \"" << Name << " FileName1 [Filename2...] [Options...]\"\n";
    if (!Full)
    {
        Out << "\"" << Name << " --help\" for displaying more information.\n"
            << endl;
        return ReturnValue_OK;
    }
    Out << "\n"
        "Options:\n"
        "    --help, -h\n"
        "        Display this help and exit.\n"
        "\n"
        "    --version\n"
        "        Display DVRescue version and exit.\n"
        "\n"
        "    --cc-format value\n"
        "        Set Closed Captions output format to value.\n"
        "        value can be 'scc', 'screen', 'srt'.\n"
        "        If there is more than one instance of this option,\n"
        "        this option is applied to the next --cc-output option.\n"
        "\n"
        "    --cc-output value | -c value\n"
        "        Store Closed Captions output to value (file name).\n"
        "        File extension must be the format name (see above)\n"
        "        if --cc-format is not provided.\n"
        "        if content is different between Dseq and/or has more than 1 field,\n"
        "        extension is prefixed by 'dseq%dseq%.' and/or 'field%field%.'.\n"
        "        There can be more than one instance of this option.\n"
        "\n"
        "    --cc-tc value\n"
        "        Set Closed Captions output start time code to value.\n"
        "        Used for SCC output.\n"
        "        value format is HH:MM:SS;FF, or 'dv' (for DV first frame time code).\n"
        "\n"
        "    --webvtt-output value | -s value\n"
        "        Store WebVTT output to value (file name).\n"
        "\n"
        "    --xml-output value | -x value\n"
        "        Store XML output to value (file name).\n"
        "\n"
        "    --merge value | -m value\n"
        "        Merge all input files into value (file name),\n"
        "        picking the best part of each file.\n"
        "\n"
        "    --merge-log value\n"
        "        Store merge log to value (file name).\n"
        "\n"
        "    --merge-output-speed\n"
        "        Report and merge frames having speed not meaning normal playback\n"
        "        Is default (it will change in the future).\n"
        "\n"
        "    --merge-ignore-speed\n"
        "        Do not report and do not merge frames having speed not\n"
        "        meaning normal playback.\n"
        "\n"
        "    --merge-output-concealed\n"
        "        Report and merge frames having all blocks concealed.\n"
        "        Is default (it will change in the future).\n"
        "\n"
        "    --merge-ignore-concealed\n"
        "        Do not report and do not merge frames having all blocks concealed.\n"
        "\n"
        "    --merge-log-missing\n"
        "        Report frames considered as missing (due to time code jump etc).\n"
        "        Is default if information output format is not CSV.\n"
        "\n"
        "    --merge-hide-missing\n"
        "        Do not report frames considered as missing (due to time code jump etc).\n"
        "        Is default if information output format is CSV.\n"
        "\n"
        "    --merge-log-intermediate\n"
        "        Display additional lines of inofrmation\n"
        "        about intermediate analysis during files merge.\n"
        "        Is default if information output format is not CSV.\n"
        "\n"
        "    --merge-hide-intermediate\n"
        "        Hide additional lines of inofrmation\n"
        "        about intermediate analysis during files merge.\n"
        "        Is default if information output format is CSV.\n"
        "\n"
        "    --csv\n"
        "        Output is in CSV format rather than human readable text.\n"
        "\n"
        "    --caption-presence-change value\n"
        "        Split XML \"frames\" elements when there is a caption presence\n"
        "        change (value = \"y\") or do not split (value = \"n\").\n"
        "        Default is \"n\".\n"
        "\n"
        "    --verbosity value | -v value\n"
        "        Verbosity of the output set to value:\n"
        "        0: no output.\n"
        "        5: summary only.\n"
        "        7: information per frame if there is a problem + summary.\n"
        "        9: information per frame + summary.\n"
        "\n"
        "    --timeout value\n"
        "        Time out limit for the pipe input (\"-\" file name) set to value (in seconds)\n"
        "\n"
        "    --capture\n"
        "        Launch capture.\n"
        "        Is the default if no --cmd option is provided.\n"
        "        Usable only if input is a device.\n"
        "\n"
        "    --in-control\n"
        "        Include an integrated command line input for controling the input.\n"
        "        Usable only if input is a device.\n"
        "\n"
        "    --list_devices\n"
        "        List detected devices and their indices.\n"
        "\n"
        "    --status\n"
        "        Provide the status (playing, stop...) of the input.\n"
        "        By default device://0 is used.\n"
        "        Usable only if input is a device.\n"
        "\n"
        "    --cmd value\n"
        "        Send a command to the input.\n"
        "        By default device://0 is used.\n"
        "        Usable only if input is a device.\n"
        "        value may be:\n"
        "        play      Set speed to 1.0 and mode to play.\n"
        "        srew      Set speed to -1.0 and mode to play.\n"
        "        stop      Set speed to 0.0 and mode to no-play.\n"
        "        rew       Set speed to -2.0 and mode to play.\n"
        "        ff        Set speed to 2.0 and mode to play.\n"
        "\n"
        "    --mode value\n"
        "        Send a command to the input with the specified mode.\n"
        "        By default device://0 is used.\n"
        "        By default value is n if speed is 0 else p.\n"
        "        Usable only if input is a device.\n"
        "        value may be:\n"
        "        n         Set mode to no-play.\n"
        "        p         Set mode to play.\n"
        "\n"
        "    --speed value\n"
        "        Send a command to the input with the specified speed (float).\n"
        "        By default device://0 is used.\n"
        "        By default value is 0 if mode is no-play else 1.\n"
        "        Usable only if input is a device.\n"
        "\n"
        "    --rewind-count value\n"
        "        Automatically rewind to last good frame and capture again,\n"
        "        value times.\n"
        "        Usable only if input is a device.\n"
        "\n"
        "    --rewind\n"
        "        Same as --rewind-count 1\n"
        "\n"
        "    --rewind-basename value\n"
        "        Base name of files storing buggy frames per take\n"
        "        Default is output file name.\n"
        "\n"
        "If no output file name is provided, XML output is displayed on console output."
        "\n"
        << endl;
    
    return ReturnValue_OK;
}

//---------------------------------------------------------------------------
return_value NameVersion(ostream& Out)
{
    Out <<
    NameVersion_Text() << ".\n"
    ;

    return ReturnValue_OK;
}

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
string NameVersion_Text()
{
    return
        "DVRescue v." Program_Version
        " (MediaInfoLib v." + MediaInfo_Version() + ")"
        " by MIPoPS"
        ;
}
