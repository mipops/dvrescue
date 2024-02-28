/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

#include "Common/SimulatorWrapper.h"
#include "ZenLib/File.h"
#include "ZenLib/ZtringListList.h"
#include "TimeCode.h"
#include <chrono>
#include <cmath>
#include <mutex>
#include <cstdlib>
#include <thread>
#include <vector>

using namespace std;
using namespace std::chrono;
using namespace ZenLib;

#if defined _WIN32 || defined WIN32
    #define SIM_PREFIX __T("C:\\Temp\\dvrescue_simulator.")
#else
    #define SIM_PREFIX __T("/tmp/dvrescue_simulator.")
#endif

//---------------------------------------------------------------------------
struct decklink_simulator
{
    int32u                      Width = {};
    int32u                      Height = {};
    uint8_t*                    Video_Buffer = {};
    size_t                      Video_Buffer_Size = {};
    uint8_t*                    Audio_Buffer = {};
    size_t                      Audio_Buffer_Size = {};
    TimeCode                    TC = {};
};

//---------------------------------------------------------------------------
struct ctl
{
    playback_mode               Mode = Playback_Mode_NotPlaying;
    float                       Speed = 0;

    // Current
    FileWrapper*                Wrapper = nullptr;
    bool                        IsCapturing = false;
    size_t                      F_Pos = 0;
    vector<File*>               F;
    mutex                       Mutex;
    steady_clock::time_point    Time_Previous_Frame;
    float                       Speed_Simulated = 0;
    int                         Speed_Simulated_Repetitions = 0;

    // I/O
    size_t                      Io_Pos = (size_t)-1;
    uint8_t*                    Buffer;
    size_t                      Buffer_Size;
    size_t                      Buffer_Offset;
    size_t                      CurrentClusterPos;
    size_t                      NextClusterPos;

    // MKV info
    size_t                      Track_Pos = 0;
    decklink_simulator          Decklink_Sim;

    // MKV parsing
    uint64_t                    Get_EB();
    bool                        UnknownSize(uint64_t Name, uint64_t Size);
    void                        ParseBuffer_Init(File& F);
    void                        ParseBuffer(File& F);

    typedef void (ctl::*call)();
    typedef call(ctl::*name)(uint64_t);

    static const size_t Levels_Max = 16;
    struct levels_struct
    {
        name SubElements;
        uint64_t Offset_End;
    };
    levels_struct Levels[Levels_Max];
    size_t Level;
    bool IsList;

    levels_struct Cluster_Levels[Levels_Max];
    size_t Cluster_Level = (size_t)-1;
    size_t Cluster_Offset = (size_t)-1;
    size_t Element_Begin_Offset;

    #define MATROSKA_ELEMENT(_NAME) \
        void _NAME(); \
        call SubElements_##_NAME(uint64_t Name);

    #define MATROSKA_ELEM_XY(_NAME, _X, _Y) \
        void _NAME##_X##_Y() { Segment_Attachments_AttachedFile_FileData_RawCookedxxx_yyy(reversibility::element::_Y, type::_X); } \
        call SubElements_##_NAME##_X##_Y(uint64_t Name);

    MATROSKA_ELEMENT(_);
    MATROSKA_ELEMENT(Segment);
    MATROSKA_ELEMENT(Segment_Cluster);
    MATROSKA_ELEMENT(Segment_Cluster_SimpleBlock);
    MATROSKA_ELEMENT(Segment_Cluster_BlockGroup);
    MATROSKA_ELEMENT(Segment_Cluster_BlockGroup_SimpleBlock);
    MATROSKA_ELEMENT(Segment_Cluster_Timestamp);
    MATROSKA_ELEMENT(Segment_Tracks);
    MATROSKA_ELEMENT(Segment_Tracks_TrackEntry);
    MATROSKA_ELEMENT(Segment_Tracks_TrackEntry_Video);
    MATROSKA_ELEMENT(Segment_Tracks_TrackEntry_Video_PixelWidth);
    MATROSKA_ELEMENT(Segment_Tracks_TrackEntry_Video_PixelHeight);
    MATROSKA_ELEMENT(Void);
};

//---------------------------------------------------------------------------
// Matroska parser

#define ELEMENT_BEGIN(_VALUE) \
ctl::call ctl::SubElements_##_VALUE(uint64_t Name) \
{ \
    switch (Name) \
    { \

#define ELEMENT_CASE(_VALUE,_NAME) \
    case 0x##_VALUE:  Levels[Level].SubElements = &ctl::SubElements_##_NAME;  return &ctl::_NAME;

#define ELEMENT_VOID(_VALUE,_NAME) \
    case 0x##_VALUE:  Levels[Level].SubElements = &ctl::SubElements_Void;  return &ctl::_NAME;


#define ELEMENT_END() \
    default:                        return SubElements_Void(Name); \
    } \
} \

ELEMENT_BEGIN(_)
ELEMENT_CASE( 8538067, Segment)
ELEMENT_END()

ELEMENT_BEGIN(Segment)
ELEMENT_CASE( F43B675, Segment_Cluster)
ELEMENT_CASE( 654AE6B, Segment_Tracks)
ELEMENT_END()

ELEMENT_BEGIN(Segment_Cluster)
ELEMENT_CASE(      20, Segment_Cluster_BlockGroup)
ELEMENT_VOID(      23, Segment_Cluster_SimpleBlock)
ELEMENT_VOID(      67, Segment_Cluster_Timestamp)
ELEMENT_END()

ELEMENT_BEGIN(Segment_Cluster_BlockGroup)
ELEMENT_VOID(      21, Segment_Cluster_BlockGroup_SimpleBlock)
ELEMENT_END()

ELEMENT_BEGIN(Segment_Tracks)
ELEMENT_CASE(      2E, Segment_Tracks_TrackEntry)
ELEMENT_END()

ELEMENT_BEGIN(Segment_Tracks_TrackEntry)
ELEMENT_CASE(      60, Segment_Tracks_TrackEntry_Video)
ELEMENT_END()

ELEMENT_BEGIN(Segment_Tracks_TrackEntry_Video)
ELEMENT_VOID(      30, Segment_Tracks_TrackEntry_Video_PixelWidth)
ELEMENT_VOID(      3A, Segment_Tracks_TrackEntry_Video_PixelHeight)
ELEMENT_END()


//---------------------------------------------------------------------------
ctl::call ctl::SubElements_Void(uint64_t /*Name*/)
{
    Levels[Level].SubElements = &ctl::SubElements_Void; return &ctl::Void;
}

//---------------------------------------------------------------------------
uint64_t ctl::Get_EB()
{
    //TEST_BUFFEROVERFLOW(1);

    uint64_t ToReturn = Buffer[Buffer_Offset];
    if (!ToReturn)
        return (uint64_t)-1; // Out of specifications, consider the value as Unlimited
    uint64_t s = 0;
    while (!(ToReturn & (((uint64_t)1) << (7 - s))))
        s++;
    ToReturn ^= (((uint64_t)1) << (7 - s));
    //TEST_BUFFEROVERFLOW(1 + s);
    uint64_t UnknownValue = (((uint64_t)1) << ((s + 1) * 7)) - 1;
    while (s)
    {
        ToReturn <<= 8;
        Buffer_Offset++;
        s--;
        ToReturn |= Buffer[Buffer_Offset];
    }
    if (ToReturn == UnknownValue)
        ToReturn = (uint64_t)-1; // Unknown size
    Buffer_Offset++;

    return ToReturn;
}

//---------------------------------------------------------------------------
bool ctl::UnknownSize(uint64_t Name, uint64_t Size)
{
    // Continue
    Levels[Level].Offset_End = Levels[Level - 1].Offset_End;
    return false;
}

//---------------------------------------------------------------------------
void ctl::ParseBuffer_Init(File& F)
{
    if (Buffer_Size < 4 || Buffer[0] != 0x1A || Buffer[1] != 0x45 || Buffer[2] != 0xDF || Buffer[3] != 0xA3)
        return;

    Buffer_Offset = 0;
    Level = 0;

    Levels[Level].Offset_End = F.Size_Get();
    Levels[Level].SubElements = &ctl::SubElements__;
    Level++;

    ParseBuffer(F);
}

//---------------------------------------------------------------------------
void ctl::ParseBuffer(File& F)
{
    while (Buffer_Offset < Levels[0].Offset_End)
    {
        Element_Begin_Offset = Buffer_Offset;
        uint64_t Name = Get_EB();
        uint64_t Size = Get_EB();
        if (Size <= Levels[Level - 1].Offset_End - Buffer_Offset)
            Levels[Level].Offset_End = Buffer_Offset + Size;
        else if (UnknownSize(Name, Size))
            break; // Problem, we stop
        call Call = (this->*Levels[Level - 1].SubElements)(Name);
        IsList = false;
        (this->*Call)();
        if (!IsList)
            Buffer_Offset = Levels[Level].Offset_End;
        if (Buffer_Offset < Levels[Level].Offset_End)
            Level++;
        else
        {
            while (Level && Buffer_Offset >= Levels[Level - 1].Offset_End)
            {
                Levels[Level].SubElements = nullptr;
                Level--;
            }
        }

        if (CurrentClusterPos == Buffer_Offset)
        {
            if (Buffer_Offset < Levels[0].Offset_End && NextClusterPos + 8 >= Buffer_Size)
            {
                for (size_t i = 0; i < Level; i++)
                    Levels[i].Offset_End -= Buffer_Offset;
                memmove(Buffer, Buffer + Buffer_Offset, Buffer_Size - Buffer_Offset);
                auto ToRead = Buffer_Offset < Levels[0].Offset_End ? Buffer_Offset : Levels[0].Offset_End;
                F.Read(Buffer + Buffer_Size - Buffer_Offset, ToRead);
                Buffer_Offset = 0;
            }

            return; // New cluster
        }
    }
}

//---------------------------------------------------------------------------
void ctl::Segment()
{
    IsList = true;
}

//---------------------------------------------------------------------------
void ctl::Segment_Cluster()
{
    IsList = true;

    CurrentClusterPos = Buffer_Offset;
    NextClusterPos = Levels[Level].Offset_End;
    Track_Pos = (size_t)-1;
}

//---------------------------------------------------------------------------
void ctl::Segment_Cluster_BlockGroup()
{
    IsList = true;
}

//---------------------------------------------------------------------------
void ctl::Segment_Cluster_BlockGroup_SimpleBlock()
{
    Segment_Cluster_SimpleBlock();
}

//---------------------------------------------------------------------------
void ctl::Segment_Cluster_SimpleBlock()
{
    Track_Pos++;
    Buffer_Offset += 4;

    auto Size = Levels[Level].Offset_End - Buffer_Offset;

    switch (Track_Pos)
    {
    case 0:
        Decklink_Sim.Video_Buffer = Buffer + Buffer_Offset;
        Decklink_Sim.Video_Buffer_Size = Size;
        break;
    case 1:
        Decklink_Sim.Audio_Buffer = Buffer + Buffer_Offset;
        Decklink_Sim.Audio_Buffer_Size = Size;
        break;
    case 2:
        Decklink_Sim.TC.FromString(string((const char*)Buffer + Buffer_Offset, Size));
        break;
    }
}

//---------------------------------------------------------------------------
void ctl::Segment_Cluster_Timestamp()
{
}

//---------------------------------------------------------------------------
void ctl::Segment_Tracks()
{
    IsList = true;
}

//---------------------------------------------------------------------------
void ctl::Segment_Tracks_TrackEntry()
{
    IsList = true;
}

//---------------------------------------------------------------------------
void ctl::Segment_Tracks_TrackEntry_Video()
{
    IsList = true;
}

//---------------------------------------------------------------------------
void ctl::Segment_Tracks_TrackEntry_Video_PixelWidth()
{
    uint32_t Data = 0;
    if (Levels[Level].Offset_End - Buffer_Offset == 1)
        Data = ((uint32_t)Buffer[Buffer_Offset]);
    if (Levels[Level].Offset_End - Buffer_Offset == 2)
        Data = (((uint32_t)Buffer[Buffer_Offset]) << 8) | ((uint32_t)Buffer[Buffer_Offset + 1]);

    Decklink_Sim.Width = Data;
}

//---------------------------------------------------------------------------
void ctl::Segment_Tracks_TrackEntry_Video_PixelHeight()
{
    uint32_t Data = 0;
    if (Levels[Level].Offset_End - Buffer_Offset == 1)
        Data = ((uint32_t)Buffer[Buffer_Offset]);
    if (Levels[Level].Offset_End - Buffer_Offset == 2)
        Data = (((uint32_t)Buffer[Buffer_Offset]) << 8) | ((uint32_t)Buffer[Buffer_Offset + 1]);

    Decklink_Sim.Height = Data;
}

//---------------------------------------------------------------------------
void ctl::Void()
{
}

//---------------------------------------------------------------------------
static std::string GetStatus(float Speed)
{
    string status;
    if (Speed == 0.0f) {
        status = "stopped";
    }
    else if (Speed == 1.0f) {
        status = "playing";
    }
    else if (Speed > 1.0f) {
        status = "fast-forwarding";
    }
    else if (Speed < 0.0f) {
        status = "rewinding";
    }
    else {
        status = "unknown";
    }

    return status;
}

//---------------------------------------------------------------------------
static Ztring MakeStatusFileName(size_t i)
{
    Ztring Result(SIM_PREFIX __T("status."));
    Result += Ztring::ToZtring(i);
    Result += __T(".txt");
    return Result;
}

//---------------------------------------------------------------------------
static Ztring MakePositionFileName(size_t i)
{
    Ztring Result(SIM_PREFIX __T("pos."));
    Result += Ztring::ToZtring(i);
    Result += __T(".txt");
    return Result;
}

//---------------------------------------------------------------------------
struct status_info
{
    playback_mode               Mode = Playback_Mode_NotPlaying;
    float                       Speed = 0;
};
static bool MakeStatusInfo(status_info& StatusInfo, size_t i)
{
    File List_F;
    if (List_F.Open(MakeStatusFileName(i), File::Access_Read))
    {
        int64u p = 0;
        char* s = (char*)&p;
        if (List_F.Read((int8u*)s, 8))
        {
            StatusInfo.Mode = s[0] == 'P' ? Playback_Mode_Playing : Playback_Mode_NotPlaying;
            StatusInfo.Speed = (float)atof(s + 1);
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------
static bool MakePosInfo(int64u& Pos, size_t i)
{
    auto Name = MakePositionFileName(i);
    File List_F;
    if (List_F.Open(Name, File::Access_Read))
    {
        char* s = (char*)&Pos;
        if (!List_F.Read((int8u*)s, 8))
            return false;
        List_F.Close();
        File::Delete(Name);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------
static ZtringListList ReadFileNames()
{
    File List_F;
    if (!List_F.Open(SIM_PREFIX __T("txt")))
        return {};
    int8u* List_C = new int8u[List_F.Size_Get()];
    List_F.Read(List_C, List_F.Size_Get());
    ZtringListList List;
    List.Separator_Set(1, __T(","));
    List.Write(Ztring().From_UTF8((char*)List_C, List_F.Size_Get()));
    return List;
}

//---------------------------------------------------------------------------
const string SimulatorWrapper::Interface = "Simulator";

//---------------------------------------------------------------------------
SimulatorWrapper::SimulatorWrapper(std::size_t DeviceIndex)
{
    auto P = new ctl;

    auto List = ReadFileNames();
    if (DeviceIndex >= List.size() || List[DeviceIndex].empty())
    {
        Ctl = nullptr;
        return;
    }

    P->Io_Pos = DeviceIndex;
    auto const& FileName = List[DeviceIndex][0];
    if (File::Exists(FileName))
    {
        P->F.push_back(new File(FileName));
        auto DotPos = FileName.rfind(__T('.'));
        bool IsMkv = DotPos != string::npos && FileName.substr(DotPos + 1) == __T("mkv");
        if (IsMkv)
            P->Buffer_Size = 16 * 1024 * 1024;
        else
            P->Buffer_Size = 120000;
        P->Buffer = new int8u[P->Buffer_Size];
        if (IsMkv)
        {
            P->F.back()->Read(P->Buffer, P->Buffer_Size);
            P->ParseBuffer_Init(*P->F.back());
        }
    }
    for (size_t i = 0;; i++)
    {
        auto FileNameExt = FileName + __T('.') + Ztring::ToZtring(i);
        if (!File::Exists(FileNameExt))
            break;
        P->F.push_back(new File(FileNameExt));
    }

    if (!P->F.empty())
    {
        int64u Pos = 0;
        if (MakePosInfo(Pos, 0))
        {
            P->F[0]->GoTo(Pos);
        }
    }

    Ctl = P;
}

//---------------------------------------------------------------------------
std::size_t SimulatorWrapper::GetDeviceCount()
{
    auto List = ReadFileNames();
    return List.size();
}

//---------------------------------------------------------------------------
std::string SimulatorWrapper::GetDeviceName(std::size_t DeviceIndex)
{
    auto List = ReadFileNames();
    if (DeviceIndex >= List.size() || List[DeviceIndex].empty())
        return {};
    return List[DeviceIndex][0].To_UTF8() + " [" + Interface + "]";
}

//---------------------------------------------------------------------------
std::string SimulatorWrapper::GetStatus()
{
    if (!Ctl)
        return {};
    auto P = (ctl*)Ctl;

    if (!P->IsCapturing)
    {
        status_info StatusInfo;
        if (MakeStatusInfo(StatusInfo, P->Io_Pos))
            return ::GetStatus(StatusInfo.Speed);
        return "unknown";
    }

    return ::GetStatus(P->Speed);
}

//---------------------------------------------------------------------------
playback_mode SimulatorWrapper::GetMode()
{
    if (!Ctl)
        return {};
    auto P = (ctl*)Ctl;

    return P->Mode;
}

//---------------------------------------------------------------------------
float SimulatorWrapper::GetSpeed()
{
    if (!Ctl)
        return {};
    auto P = (ctl*)Ctl;

    return P->Speed;
}

//---------------------------------------------------------------------------
void SimulatorWrapper::CreateCaptureSession(FileWrapper* Wrapper)
{
    auto P = (ctl*)Ctl;

    P->Wrapper = Wrapper;
}

//---------------------------------------------------------------------------
void SimulatorWrapper::StartCaptureSession()
{
    if (!Ctl)
        return;
    auto P = (ctl*)Ctl;

    P->IsCapturing = true;
}

//---------------------------------------------------------------------------
void SimulatorWrapper::StopCaptureSession()
{
    if (!Ctl)
        return;
    auto P = (ctl*)Ctl;

    P->IsCapturing = false;
}

//---------------------------------------------------------------------------
bool SimulatorWrapper::WaitForSessionEnd(uint64_t Timeout)
{
    if (!Ctl)
        return false;
    auto P = (ctl*)Ctl;

    // Check if should quit
    if (P->F.empty() || !P->IsCapturing)
        return false;

    for (;;)
    {
        P->Mutex.lock();

        // I/O
        status_info StatusInfo;
        if (MakeStatusInfo(StatusInfo, P->Io_Pos) && (StatusInfo.Mode != P->Mode || StatusInfo.Speed != P->Speed))
        {
            P->Mutex.unlock();
            SetPlaybackMode(StatusInfo.Mode, StatusInfo.Speed);
            P->Mutex.lock();
        }

        // Sleep
        auto Mode = P->Mode;
        auto Speed = P->Speed;
        auto Speed_Simulated = P->Speed_Simulated;
        P->Mutex.unlock();
        if (!Speed)
        {
            break;
        }
        auto LastFrameTheoriticalDuration = std::chrono::microseconds(abs(Speed_Simulated) < 0.3 ? 100000 : (int)(1000000.0 / (30.0 / 1.001)));
        auto Now = steady_clock::now();
        auto LastFrameDuration = duration_cast<std::chrono::microseconds>(Now - P->Time_Previous_Frame);
        if (LastFrameTheoriticalDuration > LastFrameDuration)
        {
            auto Duration = LastFrameTheoriticalDuration - LastFrameDuration;
            this_thread::sleep_for(Duration);
            P->Time_Previous_Frame = steady_clock::now();
        }
        else
            P->Time_Previous_Frame = Now;

        // In case of non playback speed
        if (Speed_Simulated > 0.0 && Speed_Simulated < 1.0)
        {
            auto Speed_Simulated_Repetitions_Max = (int)ceil(1 / Speed_Simulated);
            if (Speed_Simulated_Repetitions_Max > 4)
                Speed_Simulated_Repetitions_Max = 4;
            P->Mutex.lock();
            if (P->Speed_Simulated_Repetitions <= Speed_Simulated_Repetitions_Max)
            {
                P->Speed_Simulated_Repetitions++;
                auto& F = P->F[P->F_Pos];
                auto Position = F->Position_Get();
                if (Position < 120000)
                    F->GoTo(0, File::FromBegin);
                else
                    F->GoTo(-120000, File::FromCurrent);
            }
            else
            {
                P->Speed_Simulated_Repetitions = 0;
            }
            P->Mutex.unlock();
        }
        if (Speed_Simulated < 0.0 && Speed_Simulated > -1.0)
        {
            auto Speed_Simulated_Repetitions_Max = (int)ceil(1 / -Speed_Simulated);
            if (Speed_Simulated_Repetitions_Max > 4)
                Speed_Simulated_Repetitions_Max = 4;
            P->Mutex.lock();
            if (P->Speed_Simulated_Repetitions <= Speed_Simulated_Repetitions_Max)
            {
                P->Speed_Simulated_Repetitions++;
                auto& F = P->F[P->F_Pos];
                auto Position = F->Position_Get();
                if (Position >= F->Size_Get())
                    F->GoTo(0, File::FromEnd);
                else
                    F->GoTo(120000, File::FromCurrent);
            }
            else
            {
                P->Speed_Simulated_Repetitions = 0;
            }
            P->Mutex.unlock();
        }

        // Read next data
        P->Mutex.lock();
        if (P->NextClusterPos)
        {
            P->ParseBuffer(*P->F[0]);
            P->Wrapper->Parse_Buffer((uint8_t*)&P->Decklink_Sim, sizeof(P->Decklink_Sim));
        }
        else
        {
        auto& F = P->F[P->F_Pos];
        Mode = P->Mode;
        size_t BytesToRead = 120000;
        if (Speed_Simulated < 0)
        {
            auto Position = F->Position_Get();
            if (Position < 120000 * 2)
            {
                F->GoTo(0, File::FromBegin);
                if (Speed_Simulated <= -1.0)
                {
                    P->Mutex.unlock();
                    SetPlaybackMode(Playback_Mode_NotPlaying, 0);
                    break;
                }
                BytesToRead = 0;
            }
            else
                F->GoTo(-120000 * 2, File::FromCurrent);
        }
        auto BytesRead = F->Read(P->Buffer, BytesToRead);
        P->Mutex.unlock();

        if (BytesRead != 120000 && (Speed_Simulated <= -1.0 || Speed_Simulated >= 1.0))
            break;
        if (Mode == Playback_Mode_Playing)
        {
            uint8_t* Buffer2;
            if (Speed != 1.0 || Speed_Simulated != 1.0)
            {
                static const float ForwardRewRatio = 2;
                Buffer2 = new uint8_t[120000];
                memcpy(Buffer2, P->Buffer, 120000);
                for (int Buffer_Offset = 0; Buffer_Offset < 120000; Buffer_Offset += 80)
                    if ((P->Buffer[Buffer_Offset] & 0xE0) == 0x60 && P->Buffer[Buffer_Offset + 3] == 0x51) // Audio SCT, audio_source_control, speed near 1.0
                        Buffer2[Buffer_Offset + 3 + 3] = (Speed_Simulated > 0 ? 0x80 : 0) | int(0x20 * abs(Speed_Simulated));
                if (P->Speed_Simulated != P->Speed)
                {
                    if (abs(P->Speed_Simulated - P->Speed) <= 0.1)
                        P->Speed_Simulated = Speed;
                    else if (P->Speed_Simulated < P->Speed)
                        P->Speed_Simulated += (float)0.1 * (P->Speed_Simulated > 0 ? ForwardRewRatio : 1);
                    else if (P->Speed_Simulated > P->Speed)
                        P->Speed_Simulated -= (float)0.1 * (P->Speed_Simulated > 0 ? ForwardRewRatio : 1);
                    else
                        P->Speed_Simulated = Speed;
                }
            }
            else
                Buffer2 = P->Buffer;

            if (BytesRead != 120000)
                continue;

            P->Wrapper->Parse_Buffer(Buffer2, 120000);

            if (Buffer2 != P->Buffer)
                delete[] Buffer2;
            }
        }
    }

    SetPlaybackMode(Playback_Mode_NotPlaying, 0);
    delete[] P->Buffer;

    return false;
}

//---------------------------------------------------------------------------
void SimulatorWrapper::SetPlaybackMode(playback_mode Mode, float Speed)
{
    if (!Ctl)
        return;
    auto P = (ctl*)Ctl;

    // No update if no mode/speed change
    if (P->F_Pos >= P->F.size() || (P->IsCapturing && P->Mode == Mode && P->Speed == Speed))
        return;

    P->Mutex.lock();

    // Update
    if (P->Mode == Playback_Mode_NotPlaying && P->Speed == 0 && Speed >= -1.0 && Speed <= 1.0)
        P->Speed_Simulated = Speed; // Simulating direct target speed in the case deck is stopped and request is to no fast speed
    else
        P->Speed_Simulated = P->Speed;
    P->Mode = Mode;
    P->Speed = Speed;
    P->Time_Previous_Frame = steady_clock::now();

    // Switch to next file
    if (P->IsCapturing && P->Speed < 0)
    {
        auto SeekPos = P->F[P->F_Pos]->Position_Get();
        P->F_Pos++;
        if (P->F_Pos >= P->F.size())
            P->F_Pos = 0;
        P->F[P->F_Pos]->GoTo(SeekPos);
    }

    // I/O
    if (P->Io_Pos != (size_t)-1)
    {
        auto Status = (Mode == Playback_Mode_Playing ? 'P' : 'N' ) + ::to_string(P->Speed);
        File List_F;
        if (List_F.Open(MakeStatusFileName(P->Io_Pos), File::Access_Write))
        {
            List_F.Truncate();
            List_F.Write((int8u*)Status.c_str(), Status.size());
        }
    }

    P->Mutex.unlock();
}

//---------------------------------------------------------------------------
SimulatorWrapper::~SimulatorWrapper()
{
    auto P = (ctl*)Ctl;

    // I/O
    if (P->Io_Pos != (size_t)-1 && P->F_Pos < P->F.size())
    {
        auto& F = P->F[P->F_Pos];
        int64u Pos = F->Position_Get();
        File List_F;
        if (List_F.Open(MakePositionFileName(P->Io_Pos), File::Access_Write))
        {
            List_F.Truncate();
            List_F.Write((int8u*)&Pos, 8);
        }
    }

    delete P;
}
