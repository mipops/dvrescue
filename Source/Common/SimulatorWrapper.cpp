/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

#include "Common/SimulatorWrapper.h"
#include "ZenLib/File.h"
#include "ZenLib/ZtringListList.h"
#include <chrono>
#include <mutex>
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

    // I/O
    size_t                      Io_Pos = (size_t)-1;
};

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
        P->F.push_back(new File(FileName));
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
    return List[DeviceIndex][0].To_UTF8() + " (Simulator)";
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
void SimulatorWrapper::WaitForSessionEnd()
{
    if (!Ctl)
        return;
    auto P = (ctl*)Ctl;

    // Check if should quit
    if (P->F.empty() || !P->IsCapturing)
        return;

    int8u* Buffer = new int8u[120000];
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
        P->Mutex.unlock();
        if (!Speed)
        {
            break;
        }
        auto LastFrameTheoriticalDuration = std::chrono::microseconds((int)(1000000.0 / (30.0 / 1.001) / abs(Speed)));
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

        // Read next data
        P->Mutex.lock();
        auto& F = P->F[P->F_Pos];
        Mode = P->Mode;
        if (P->Speed < 0)
        {
            auto Position = F->Position_Get();
            if (Position < 120000 * 2)
            {
                F->GoTo(0, File::FromBegin);
                P->Mutex.unlock();
                SetPlaybackMode(Playback_Mode_NotPlaying, 0);
                break;
                //continue;
            }
            F->GoTo(-120000 * 2, File::FromCurrent);
        }
        auto BytesRead = F->Read(Buffer, 120000);
        P->Mutex.unlock();

        if (BytesRead != 120000)
            break;
        if (Mode == Playback_Mode_Playing)
            P->Wrapper->Parse_Buffer(Buffer, 120000);
    }

    SetPlaybackMode(Playback_Mode_NotPlaying, 0);
    delete[] Buffer;
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
    P->Mode = Mode;
    P->Speed = Speed;
    P->Time_Previous_Frame = steady_clock::now();

    // Switch to next file
    if (P->IsCapturing)
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
    if (P->Io_Pos != (size_t)-1)
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
