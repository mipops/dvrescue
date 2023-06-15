/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#include "Common/Sony9PinWrapper.h"
#include "serial/serial.h"

#include <time.h>

//---------------------------------------------------------------------------
// Fake QSerialPort API
// TODO: Add custom serial API support to Sony9Pin
//---------------------------------------------------------------------------
#define QT_VERSION 1
class QSerialPort
{
    public:
    QSerialPort(serial::Serial* backend = nullptr) : _backend(backend) {};

    void attach(serial::Serial* backend) { _backend = backend; };
    size_t read(const char* buffer, size_t size) { return _backend ? _backend->read((uint8_t*)buffer, size) : 0; };
    size_t write(const char* buffer, size_t size) { return _backend ? _backend->write((uint8_t*)buffer, size) : 0; };
    void flush() { if (_backend) _backend->flush(); };
    size_t bytesAvailable() { return _backend ? _backend->available() : 0; };
    bool waitForBytesWritten() { return true; };
    bool waitForReadyRead(size_t) { return true; };

    static const size_t Baud38400 = 38400;

    private:
    serial::Serial* _backend;
};

//---------------------------------------------------------------------------
#include <Sony9PinRemote.h>

using namespace std;

//---------------------------------------------------------------------------
static vector<serial::PortInfo> Devices;
static serial::Serial SerialPort;
static QSerialPort SerialPortWrapper;
static Sony9PinRemote::Controller Deck;

//---------------------------------------------------------------------------
void Sony9PinWrapper::Init()
{
    Devices.clear();
    Devices = serial::list_ports();
}

//---------------------------------------------------------------------------
Sony9PinWrapper::Sony9PinWrapper(size_t DeviceIndex)
{
    static vector<serial::PortInfo> DeviceList = serial::list_ports();

    if (DeviceIndex >= DeviceList.size())
        throw error("Unable to find control port.");

    // Config
    serial::Timeout Timeout=serial::Timeout::simpleTimeout(3000);

    SerialPort.setPort(DeviceList[DeviceIndex].port);
    SerialPort.setParity(serial::parity_odd);
    SerialPort.setTimeout(Timeout);
    SerialPort.setBaudrate(Sony9PinSerial::BAUDRATE);

    // Open
    SerialPort.open();
    if (!SerialPort.isOpen())
        throw error("Unable to open control port.");

    SerialPortWrapper.attach(&SerialPort);
    Deck.attach(SerialPortWrapper);
}

//---------------------------------------------------------------------------
Sony9PinWrapper::Sony9PinWrapper(string DeviceName)
{
    // Config
    serial::Timeout Timeout=serial::Timeout::simpleTimeout(3000);

    SerialPort.setPort(DeviceName);
    SerialPort.setParity(serial::parity_odd);
    SerialPort.setTimeout(Timeout);
    SerialPort.setBaudrate(Sony9PinSerial::BAUDRATE);

    // Open
    SerialPort.open();
    if (!SerialPort.isOpen())
        throw error("Unable to open control port.");

    SerialPortWrapper.attach(&SerialPort);
    Deck.attach(SerialPortWrapper);
}

//---------------------------------------------------------------------------
Sony9PinWrapper::~Sony9PinWrapper()
{
    if (SerialPort.isOpen())
        SerialPort.close();
}

//---------------------------------------------------------------------------
bool Ack()
{
    if (!Deck.ack() && (Deck.is_nak_unknown_command() ||
                        Deck.is_nak_checksum_error() ||
                        Deck.is_nak_parity_error() ||
                        Deck.is_nak_buffer_overrun() ||
                        Deck.is_nak_framing_error() ||
                        Deck.is_nak_timeout()))
        return false;

    return true;
}

//---------------------------------------------------------------------------
size_t Sony9PinWrapper::GetDeviceCount()
{
    Init();

    return Devices.size();
}

//---------------------------------------------------------------------------
string Sony9PinWrapper::GetDeviceName(size_t DeviceIndex)
{
    Init();

    if (DeviceIndex >= Devices.size())
        return "";

    return Devices[DeviceIndex].port;
}

//---------------------------------------------------------------------------
size_t Sony9PinWrapper::GetDeviceIndex(const string& DeviceName)
{
    Init();

    for (size_t Pos = 0; Pos < Devices.size(); Pos++)
    {
        if (Devices[Pos].port == DeviceName)
            return Pos;
    }

    return (size_t)-1;
}

//---------------------------------------------------------------------------
playback_mode Sony9PinWrapper::GetMode()
{
    Deck.status_sense();
    if (Deck.parse_until(1000))
        return Deck.is_playing() ? Playback_Mode_Playing : Playback_Mode_NotPlaying;

    return Playback_Mode_NotPlaying;
}

//---------------------------------------------------------------------------
std::string Sony9PinWrapper::GetStatus()
{
    Deck.status_sense();
    if (Deck.parse_until(1000))
    {
        if (Deck.is_playing())
            return Deck.is_reverse() ? "playing (reverse)" : "playing";
        else if (Deck.is_paused())
            return "paused";
        else if (Deck.is_fast_forward())
            return "fast-forwarding";
        else if (Deck.is_fast_reverse())
            return "rewinding";
        else if (Deck.is_stopping())
            return "stopped";
    }

    return "unknown";
}

//---------------------------------------------------------------------------
float Sony9PinWrapper::GetSpeed()
{
    // Can't retrive speed from sony9pin, guessing most-plausible value
    Deck.status_sense();
    if (Deck.parse_until(1000))
    {
        if (Deck.is_playing())
            return Deck.is_reverse() ? -1.0f : 1.0f;
        else if (Deck.is_fast_forward())
            return 2.0f;
        else if (Deck.is_fast_reverse())
            return -2.0f;
    }

    return 0.0f;
}

//---------------------------------------------------------------------------
void Sony9PinWrapper::SetPlaybackMode(playback_mode Mode, float Speed)
{
    switch (Mode)
    {
        case Playback_Mode_Playing:
            if (Speed > 0.0f)
                Deck.play();
            else if (Speed < 0.0f)
                Deck.rewind();
            else
                Deck.stop();
        break;
        case Playback_Mode_NotPlaying:
            if (Speed > 1.0f)
                Deck.fast_forward();
            if (Speed > 0.0f)
                Deck.play();
            else if (Speed < -1.0f)
                Deck.fast_reverse();
            else if (Speed < 0.0f)
                Deck.rewind();
            else
                Deck.stop();
        break;
    }

    Deck.parse_until(1000);
}
