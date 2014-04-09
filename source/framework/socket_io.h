/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file socket_io.h is part of RPiCast project

 RPiCast is a free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************/

#ifndef SOCKET_H_
#define SOCKET_H_

#include "utils.h"
#include "buffer.h"
#include "basepipe.h"
#include "worker.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET_PORT 88888
#define UDP_PACKET_MAXSIZE ((16 * 1024))

class SocketInput;
class SocketOutput;

class SocketDevice: public ADevice
{

public:
    SocketDevice(std::string name, BasePipe* pipe);
    virtual ~SocketDevice();
    virtual VC_STATUS Initialize();
    virtual VC_STATUS Uninitialize();
    virtual InputPort* Input(int portno = 0);
    virtual OutputPort* Output(int portno = 0);
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual VC_STATUS SendCommand(VC_CMD cmd);

private:
    InputPort* m_input;
    OutputPort* m_output;
};

class SocketInDevice: public SocketDevice
{
public:
    SocketInDevice(std::string name, BasePipe* pipe);
    virtual ~SocketInDevice()
    {
    }
    virtual VC_STATUS SendCommand(VC_CMD cmd);
private:
    InputPort* m_socket_input;
};

class SocketOutDevice: public SocketDevice
{
public:
    SocketOutDevice(std::string name, BasePipe* pipe, const char* addr = "27.0.0.1");
    virtual ~SocketOutDevice()
    {
    }
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual VC_STATUS SendCommand(VC_CMD cmd);

private:
    OutputPort* m_socket_output;
    const char* m_server_addr;
    bool m_device_ready;
};

class SocketInput: public WorkerThread, public InputPort
{
public:
    SocketInput(std::string name, ADevice* device, const char* ip = "127.0.0.1", int port = SOCKET_PORT);
    virtual ~SocketInput();
    virtual Buffer* GetEmptyBuffer();
    virtual VC_STATUS ReceiveBuffer(Buffer* buf);

private:
    /* read task */
    virtual void Task();

    int m_handle;
    struct sockaddr_in m_server_addr;
};

class SocketOutput: public OutputPort
{
public:
    SocketOutput(std::string name, ADevice* device, const char* ip = "127.0.0.1", int port = SOCKET_PORT);
    virtual ~SocketOutput();
    virtual VC_STATUS PushBuffer(Buffer* buf);
    virtual Buffer* GetBuffer();

private:
    int m_handle;
    struct sockaddr_in m_server_addr;
    struct sockaddr_in m_client_addr;
};

#endif /* SOCKET_H_ */
