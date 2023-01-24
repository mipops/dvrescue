/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/LinuxWrapper.h"

#include <ctime>
#include <queue>
#include <vector>
#include <cstring>
#include <sstream>
#include <sys/poll.h>
#include <libavc1394/rom1394.h>
#include <libavc1394/avc1394.h>
#include <libavc1394/avc1394_vcr.h>

#include <iostream>

using namespace std;

#define CTLVCR0 AVC1394_CTYPE_CONTROL | AVC1394_SUBUNIT_TYPE_TAPE_RECORDER | AVC1394_SUBUNIT_ID_0

//---------------------------------------------------------------------------
struct frame
{
    size_t Size;
    uint8_t* Data;
};

vector<LinuxWrapper::device> Devices;
mutex ProcessFrameLock;

atomic<time_t> LastInput;

queue<frame> FrameBuffer;
mutex FrameBufferLock;


//---------------------------------------------------------------------------
static int ReceiveFrame(unsigned char* Data, int Lenght, int, void *UserData)
{
    const lock_guard<mutex> Lock(ProcessFrameLock);
    LastInput = time(NULL);

    frame Cur;
    Cur.Size = Lenght;
    Cur.Data = new uint8_t[Lenght];

    memcpy((void*)Cur.Data, (const void*) Data, Lenght);
    FrameBufferLock.lock();
    FrameBuffer.push(Cur);
    FrameBufferLock.unlock();

    return 0;
}


//---------------------------------------------------------------------------
void LinuxWrapper::Init()
{
    Devices.clear();

    raw1394handle_t Handle = raw1394_new_handle();
    if (!Handle)
        throw error("unable to create raw1394 handle");

    int Ports = raw1394_get_port_info(Handle, nullptr, 0);
    raw1394_destroy_handle(Handle);

    for (int Port = 0; Port < Ports; Port++)
    {
         Handle = raw1394_new_handle_on_port(Port);
         if (!Handle)
             continue;

         int Nodes = raw1394_get_nodecount(Handle);
         for (int Node = 0; Node < Nodes - 1 /* last node is the control node */; Node++)
         {
             rom1394_directory Directory;
             if (rom1394_get_directory(Handle, Node, &Directory) >= 0 &&
                 rom1394_get_node_type(&Directory) == ROM1394_NODE_TYPE_AVC &&
                 avc1394_check_subunit_type(Handle, Node, AVC1394_SUBUNIT_TYPE_VCR))
             {
                 rom1394_get_directory(Handle, Node, &Directory);
                 //TODO: it is mapping standard?
                 string Vendor = Directory.nr_textual_leafs > 0 ? Directory.textual_leafs[0] : "Unknown vendor";
                 string Model = Directory.nr_textual_leafs > 1 ? Directory.textual_leafs[1] : "Unknown model";
                 octlet_t UUID = rom1394_get_guid(Handle, Node);

                 Devices.push_back(device(Port, (nodeid_t)Node, UUID, Vendor, Model));
                 rom1394_free_directory(&Directory);
                 break; //TODO: Handle more than one vcr device per port?
             }
             rom1394_free_directory(&Directory);
        }
        raw1394_destroy_handle(Handle);
    }
}

//---------------------------------------------------------------------------
LinuxWrapper::LinuxWrapper(size_t DeviceIndex)
{
    Init();

    if (DeviceIndex >= Devices.size())
        throw error("Device not found.");

    Port = Devices[DeviceIndex].Port;
    Node = Devices[DeviceIndex].Node;
    UUID = Devices[DeviceIndex].UUID;

    //TODO: make code handle bus-reset
    (CtlHandle = raw1394_new_handle());
    if (!CtlHandle)
        throw error("Unable to create raw1394 handle.");

    int Ports = raw1394_get_port_info(CtlHandle, nullptr, 0);
    if (Port >= Ports)
    {
        raw1394_destroy_handle(CtlHandle);
        throw error("Device not found.");
    }

    if (raw1394_set_port(CtlHandle, Port) < 0)
    {
        raw1394_destroy_handle(CtlHandle);
        throw error("Unable to connect to device.");
    }

    iec61883_cmp_normalize_output(CtlHandle, 0xffc0 | Node);
}

//---------------------------------------------------------------------------
LinuxWrapper::LinuxWrapper(string DeviceID)
{
    Init();

    uint64_t ID = stoull(DeviceID, NULL, 16);
    if(!ID)
        throw error("Invalid device ID.");

    size_t DeviceIndex = (size_t)-1;
    for (size_t Pos = 0; Pos < Devices.size(); Pos++)
    {
        if (Devices[Pos].UUID == ID)
        {
            DeviceIndex = Pos;
            break;
        }
    }

    if (DeviceIndex == (size_t)-1)
        throw error("Device not found.");

    Port = Devices[DeviceIndex].Port;
    Node = Devices[DeviceIndex].Node;
    UUID = Devices[DeviceIndex].UUID;

    //TODO: make code handle bus-reset
    (CtlHandle = raw1394_new_handle());
    if (!CtlHandle)
        throw error("Unable to create raw1394 handle.");

    int Ports = raw1394_get_port_info(CtlHandle, nullptr, 0);
    if (Port >= Ports)
    {
        raw1394_destroy_handle(CtlHandle);
        throw error("Device not found.");
    }

    if (raw1394_set_port(CtlHandle, Port) < 0)
    {
        raw1394_destroy_handle(CtlHandle);
        throw error("Unable to connect to device.");
    }

    iec61883_cmp_normalize_output(CtlHandle, 0xffc0 | Node);
}

//---------------------------------------------------------------------------
LinuxWrapper::~LinuxWrapper()
{
    StopCaptureSession();

    if (CtlHandle)
        raw1394_destroy_handle(CtlHandle);
}

//---------------------------------------------------------------------------
size_t LinuxWrapper::GetDeviceCount()
{
    Init();

    return Devices.size();
}

//---------------------------------------------------------------------------
string LinuxWrapper::GetDeviceName(size_t DeviceIndex)
{
    Init();

    if (DeviceIndex >= Devices.size())
        return "";

    return Devices[DeviceIndex].Vendor + " " + Devices[DeviceIndex].Model;
}

//---------------------------------------------------------------------------
string LinuxWrapper::GetDeviceName(const string& DeviceID)
{
    Init();

    uint64_t ID = stoull(DeviceID, NULL, 16);
    if(!ID)
        return "";

    for (size_t Pos = 0; Pos < Devices.size(); Pos++)
    {
        if (Devices[Pos].UUID == ID)
            return Devices[Pos].Vendor + " " + Devices[Pos].Model;
    }

    return "";
}

//---------------------------------------------------------------------------
string LinuxWrapper::GetDeviceID(size_t DeviceIndex)
{
    Init();

    if (DeviceIndex >= Devices.size())
        return "";

    stringstream Stream;
    Stream << "0x" << hex << (uint64_t)Devices[DeviceIndex].UUID;

    return Stream.str();
}

//---------------------------------------------------------------------------
size_t LinuxWrapper::GetDeviceIndex(const string& DeviceID)
{
    Init();

    uint64_t ID = stoull(DeviceID, NULL, 16);
    if(!ID)
        return (size_t)-1;

    for (size_t Pos = 0; Pos < Devices.size(); Pos++)
    {
        if (Devices[Pos].UUID == ID)
            return Pos;
    }

    return (size_t)-1;
}

//---------------------------------------------------------------------------
playback_mode LinuxWrapper::GetMode()
{
    quadlet_t Status = avc1394_vcr_status(CtlHandle, Node);
    if (AVC1394_MASK_OPCODE(Status) == AVC1394_VCR_RESPONSE_TRANSPORT_STATE_PLAY)
    {
        if (AVC1394_MASK_OPERAND0(Status) >= AVC1394_VCR_OPERAND_PLAY_SLOWEST_FORWARD &&
            AVC1394_MASK_OPERAND0(Status) <= AVC1394_VCR_OPERAND_PLAY_FASTEST_FORWARD)
            return Playback_Mode_Playing;
    }

    return Playback_Mode_NotPlaying;
}

//---------------------------------------------------------------------------
std::string LinuxWrapper::GetStatus()
{
    if (!CtlHandle || Node == (nodeid_t)-1)
        return "invalid";

    quadlet_t Status = avc1394_vcr_status(CtlHandle, Node);

    if (AVC1394_MASK_OPCODE(Status) == AVC1394_VCR_RESPONSE_TRANSPORT_STATE_PLAY)
    {
        if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_PLAY_REVERSE_PAUSE ||
            AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_PLAY_FORWARD_PAUSE)
            return "paused";
        if (AVC1394_MASK_OPERAND0(Status) >= AVC1394_VCR_OPERAND_PLAY_FAST_FORWARD_1 &&
            AVC1394_MASK_OPERAND0(Status) <= AVC1394_VCR_OPERAND_PLAY_FASTEST_FORWARD)
            return "playing (fast-forward)";
        if ((AVC1394_MASK_OPERAND0(Status) >= AVC1394_VCR_OPERAND_PLAY_NEXT_FRAME &&
             AVC1394_MASK_OPERAND0(Status) <= AVC1394_VCR_OPERAND_PLAY_X1_FORWARD) ||
             AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_PLAY_FORWARD)
            return "playing";
        if (AVC1394_MASK_OPERAND0(Status) >= AVC1394_VCR_OPERAND_PLAY_FAST_REVERSE_1 &&
            AVC1394_MASK_OPERAND0(Status) <= AVC1394_VCR_OPERAND_PLAY_FASTEST_REVERSE)
            return "playing (fast-reverse)";
        if ((AVC1394_MASK_OPERAND0(Status) >= AVC1394_VCR_OPERAND_PLAY_PREVIOUS_FRAME &&
             AVC1394_MASK_OPERAND0(Status) <= AVC1394_VCR_OPERAND_PLAY_X1_REVERSE) ||
             AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_PLAY_REVERSE)
            return "playing (reverse)";
        else
            return "unknown";
    }
    else if (AVC1394_MASK_OPCODE(Status) == AVC1394_VCR_RESPONSE_TRANSPORT_STATE_WIND)
    {
        if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_WIND_FAST_FORWARD)
            return "fast-forwarding";
        else if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_WIND_REWIND ||
                 AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_WIND_HIGH_SPEED_REWIND)
            return "rewinding";
        else if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_WIND_STOP)
            return "stopped";
        else
            return "unknown";
    }

    return "unknown";
}

//---------------------------------------------------------------------------
float LinuxWrapper::GetSpeed()
{
    // Can't retrive speed from libavc1394, guessing most-plausible value
    if (!CtlHandle || Node == (nodeid_t)-1)
        return 0.0f;

    quadlet_t Status = avc1394_vcr_status(CtlHandle, Node);
    if (AVC1394_MASK_OPCODE(Status) == AVC1394_VCR_RESPONSE_TRANSPORT_STATE_PLAY)
    {
        if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_PLAY_REVERSE_PAUSE ||
            AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_PLAY_FORWARD_PAUSE)
            return 0.0f;
        if (AVC1394_MASK_OPERAND0(Status) >= AVC1394_VCR_OPERAND_PLAY_FAST_FORWARD_1 &&
            AVC1394_MASK_OPERAND0(Status) <= AVC1394_VCR_OPERAND_PLAY_FASTEST_FORWARD)
            return 2.0f;
        if (AVC1394_MASK_OPERAND0(Status) >= AVC1394_VCR_OPERAND_PLAY_NEXT_FRAME &&
            AVC1394_MASK_OPERAND0(Status) <= AVC1394_VCR_OPERAND_PLAY_X1_FORWARD)
             return 0.5f;
        if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_PLAY_FORWARD)
            return 1.0f;
        if (AVC1394_MASK_OPERAND0(Status) >= AVC1394_VCR_OPERAND_PLAY_FAST_REVERSE_1 &&
            AVC1394_MASK_OPERAND0(Status) <= AVC1394_VCR_OPERAND_PLAY_FASTEST_REVERSE)
            return -2.0f;
        if (AVC1394_MASK_OPERAND0(Status) >= AVC1394_VCR_OPERAND_PLAY_PREVIOUS_FRAME &&
            AVC1394_MASK_OPERAND0(Status) <= AVC1394_VCR_OPERAND_PLAY_X1_REVERSE)
            return -0.5f;
        if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_PLAY_REVERSE)
            return -1.0f;
    }
    else if (AVC1394_MASK_OPCODE(Status) == AVC1394_VCR_RESPONSE_TRANSPORT_STATE_WIND)
    {
        if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_WIND_FAST_FORWARD)
            return 2.0f;
        else if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_WIND_REWIND ||
                 AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_WIND_HIGH_SPEED_REWIND)
            return -2.0f;
        else if (AVC1394_MASK_OPERAND0(Status) == AVC1394_VCR_OPERAND_WIND_STOP)
            return 0.0f;
    }

    return 0.0f;
}

//---------------------------------------------------------------------------
void LinuxWrapper::CreateCaptureSession(FileWrapper* Wrapper_)
{
    Wrapper = Wrapper_;

    CaptureHandle = raw1394_new_handle_on_port(Port);
    if (CaptureHandle)
    {
        Channel = iec61883_cmp_connect(CaptureHandle, Node, &OutPlug, raw1394_get_local_id(CaptureHandle), &InPlug, &Bandwidth);
        if (Channel < 0) // try broadcast channel if connect failed
            Channel = 63;
        Frame = iec61883_dv_fb_init(CaptureHandle, ReceiveFrame, nullptr);
    }
}

//---------------------------------------------------------------------------
void LinuxWrapper::StartCaptureSession()
{
    if (Frame && iec61883_dv_fb_start(Frame, Channel) == 0)
    {
        Raw1394PoolingThread = new thread([this]() {
            struct pollfd Desc = {
                fd: raw1394_get_fd(CaptureHandle),
                events: POLLIN | POLLERR | POLLHUP | POLLPRI,
                revents: 0
            };
            int Result = 0;
            do
            {
                {
                    const lock_guard<mutex> Lock(ProcessFrameLock);
                }
                if (poll(&Desc, 1, 100) > 0 && (Desc.revents & POLLIN))
                    Result = raw1394_loop_iterate(CaptureHandle);
            }
            while (Result == 0 && !Raw1394PoolingThread_Terminate);
        });

        ProcessFrameThread = new thread([this]() {
            do
            {
                FrameBufferLock.lock();
                if (Wrapper && !FrameBuffer.empty())
                {
                    frame Cur = FrameBuffer.back();
                    FrameBuffer.pop();

                    Wrapper->Parse_Buffer(Cur.Data, Cur.Size);
                    delete[] Cur.Data;
                }
                FrameBufferLock.unlock();

                this_thread::yield();
            }
            while (!Raw1394PoolingThread_Terminate);
        });
    }
}

//---------------------------------------------------------------------------
void LinuxWrapper::StopCaptureSession()
{
    // TODO: Separate Stop/Destroy logics
    if (Raw1394PoolingThread)
    {
        Raw1394PoolingThread_Terminate = true;
        if (Raw1394PoolingThread->joinable())
            Raw1394PoolingThread->join();

        delete Raw1394PoolingThread;
        Raw1394PoolingThread=nullptr;
        Raw1394PoolingThread_Terminate = false;
    }

    if (ProcessFrameThread)
    {
        ProcessFrameThread_Terminate = true;
        if (ProcessFrameThread->joinable())
            ProcessFrameThread->join();

        delete ProcessFrameThread;
        ProcessFrameThread=nullptr;
        ProcessFrameThread_Terminate = false;
    }

    FrameBufferLock.lock();
    while (!FrameBuffer.empty())
    {
        delete[] FrameBuffer.back().Data;
        FrameBuffer.pop();
    }
    FrameBufferLock.unlock();

     if (Wrapper)
     {
         Wrapper = nullptr;
     }

     if (Frame)
     {
         iec61883_dv_fb_close(Frame);
         Frame = 0;
     }

    if (Channel >= 0 && Channel != 63)
    {
        iec61883_cmp_disconnect(CaptureHandle, Node, OutPlug, raw1394_get_local_id(CaptureHandle), InPlug, Channel, Bandwidth);
        Channel = -1;
    }

    if (CaptureHandle)
    {
        raw1394_destroy_handle(CaptureHandle);
        CaptureHandle=nullptr;
    }

    Bandwidth = -1;
    OutPlug = -1;
    InPlug = -1;
}

//---------------------------------------------------------------------------
bool LinuxWrapper::WaitForSessionEnd(uint64_t Timeout)
{
    LastInput = time(NULL);
    do
    {
        if (Timeout)
        {
            if (difftime(time(NULL), LastInput) > Timeout)
                return true;
        }
        this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    while (GetSpeed() != 0.0f);

    return false;
}

//---------------------------------------------------------------------------
void LinuxWrapper::SetPlaybackMode(playback_mode Mode, float Speed)
{
    switch (Mode)
    {
        case Playback_Mode_Playing:
        {
            uint8_t Code;
            if (Speed >= 2.0f) // fastest play mode
                Code=AVC1394_VCR_OPERAND_PLAY_FASTEST_FORWARD;
            else if (Speed > 1.0f) // 2x play
                Code=AVC1394_VCR_OPERAND_PLAY_FAST_FORWARD_6;
            else if (Speed == 1.0f) // normal speed
                Code=AVC1394_VCR_OPERAND_PLAY_X1_FORWARD;
            else if (Speed > 0.0f) // slow speed
                Code=AVC1394_VCR_OPERAND_PLAY_SLOW_FORWARD_6;
            else if (Speed == 0.0f)
                Code = AVC1394_VCR_OPERAND_PLAY_FORWARD_PAUSE; //TODO: Use PLAY_REVERSE_PAUSE if current direction is reverse
            else if (Speed > -1.0f) // normal speed, reverse
                Code=AVC1394_VCR_OPERAND_PLAY_SLOW_REVERSE_6;
            else if (Speed == -1.0f) // normal speed, reverse
                Code=AVC1394_VCR_OPERAND_PLAY_X1_REVERSE;
            else if (Speed > -2.0f) // 2x reverse play
                Code=AVC1394_VCR_OPERAND_PLAY_FAST_REVERSE_6;
            else if (Speed <= -2.0f) // fastest reverse play mode
                Code=AVC1394_VCR_OPERAND_PLAY_FASTEST_REVERSE;

            avc1394_send_command(CtlHandle, Node, CTLVCR0 | AVC1394_VCR_COMMAND_PLAY | Code);
            break;
        }
        case Playback_Mode_NotPlaying:
        {
            if (Speed == 0.0f) // stop
                avc1394_send_command(CtlHandle, Node, CTLVCR0 | AVC1394_VCR_COMMAND_WIND | AVC1394_VCR_OPERAND_WIND_STOP);
            else if (Speed > 0.0f) // fast-forward
                avc1394_send_command(CtlHandle, Node, CTLVCR0 | AVC1394_VCR_COMMAND_WIND | AVC1394_VCR_OPERAND_WIND_FAST_FORWARD);
            else if (Speed < 0.0f) // rewind
                avc1394_send_command(CtlHandle, Node, CTLVCR0 | AVC1394_VCR_COMMAND_WIND | AVC1394_VCR_OPERAND_WIND_REWIND);
            break;
        }
    }
}
