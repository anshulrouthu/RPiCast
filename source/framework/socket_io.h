/*
 * socket.h
 *
 *  Created on: Mar 9, 2014
 *      Author: anshul
 */

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
    virtual ~SocketInDevice(){}
    virtual VC_STATUS SendCommand(VC_CMD cmd);
private:
    InputPort* m_socket_input;
};

class SocketOutDevice: public SocketDevice
{
public:
    SocketOutDevice(std::string name, BasePipe* pipe, const char* addr = "27.0.0.1");
    virtual ~SocketOutDevice(){}
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
    int m_client_handle;
    struct sockaddr_in m_server_addr;
    struct sockaddr_in m_client_addr;
};

class SocketOutput: public OutputPort
{
public:
    SocketOutput(std::string name, ADevice* device, const char* ip = "127.0.0.1", int port = SOCKET_PORT);
    virtual ~SocketOutput();
    virtual VC_STATUS PushBuffer(Buffer* buf);
    virtual Buffer* GetBuffer();

private:
    bool IsConnected();

    int m_handle;
    int m_server_handle;
    struct sockaddr_in m_server_addr;
    struct sockaddr_in m_client_addr;
    bool m_connected;
};

#endif /* SOCKET_H_ */
