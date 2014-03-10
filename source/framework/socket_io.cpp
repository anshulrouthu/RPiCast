/*
 * socket.cpp
 *
 *  Created on: Mar 9, 2014
 *      Author: anshul
 */

#include "socket_io.h"

SocketDevice::SocketDevice(std::string name, BasePipe* pipe) :
    ADevice(name, pipe),
    m_input(NULL ),
    m_output(NULL )
{
}

SocketDevice::~SocketDevice()
{
}

VC_STATUS SocketDevice::Initialize()
{
    DBG_MSG("Enter");
    m_input = new InputPort("SockDev_Input", this);
    m_output = new OutputPort("SockDev_Output", this);

    return (VC_SUCCESS);
}

VC_STATUS SocketDevice::Uninitialize()
{
    delete m_input;
    delete m_output;
    return (VC_SUCCESS);
}

InputPort* SocketDevice::Input(int portno)
{
    return (m_input);
}

OutputPort* SocketDevice::Output(int portno)
{
    return (m_output);
}

VC_STATUS SocketDevice::Notify(VC_EVENT* evt)
{
    return (VC_SUCCESS);
}

VC_STATUS SocketDevice::SendCommand(VC_CMD cmd)
{
    return (VC_SUCCESS);
}

SocketInDevice::SocketInDevice(std::string name, BasePipe* pipe) :
    SocketDevice(name, pipe),
    m_socket_input(NULL )
{
    DBG_MSG("Enter");
}

VC_STATUS SocketInDevice::SendCommand(VC_CMD cmd)
{
    DBG_MSG("Enter");
    switch (cmd)
    {
    case VC_CMD_START:
        m_socket_input = new SocketInput(m_name + "_Input", this);
        break;
    case VC_CMD_STOP:
        delete m_socket_input;
        break;
    }
    return (VC_SUCCESS);
}

SocketOutDevice::SocketOutDevice(std::string name, BasePipe* pipe, const char* addr) :
    SocketDevice(name, pipe),
    m_socket_output(NULL ),
    m_server_addr(addr)
{
    DBG_MSG("Enter");
}

VC_STATUS SocketOutDevice::SendCommand(VC_CMD cmd)
{
    DBG_MSG("Enter");
    switch (cmd)
    {
    case VC_CMD_START:
        m_socket_output = new SocketOutput(m_name + "_Input", this);
        DBG_CHECK(!m_socket_output, return (VC_FAILURE), "Error: Creating socket out device");
        break;
    case VC_CMD_STOP:
        delete m_socket_output;
        break;
    }
    return (VC_SUCCESS);
}

VC_STATUS SocketOutDevice::Notify(VC_EVENT* evt)
{
    while (Input()->IsBufferAvailable())
    {
        Buffer* buf = Input()->GetFilledBuffer();
        m_socket_output->PushBuffer(buf);
        Input()->RecycleBuffer(buf);
    }
    return (VC_SUCCESS);
}

SocketInput::SocketInput(std::string name, ADevice* device, const char* addr, int port) :
    InputPort(name, device),
    m_handle(0),
    m_client_handle(0)
{
    DBG_MSG("Enter");
    m_handle = socket(2, SOCK_STREAM, IPPROTO_TCP);
    DBG_CHECK(m_handle < 0, return, "Error: Creating socket");

    bzero((char *) &m_server_addr, sizeof(m_server_addr));
    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_addr.s_addr = INADDR_ANY;
    m_server_addr.sin_port = htons(port);

    int err = bind(m_handle, (struct sockaddr *) &m_server_addr, sizeof(m_server_addr));
    DBG_CHECK(err < 0, return, "Error(%d): Unable to bind socket", err);
    listen(m_handle, 5);

    Start();
}

SocketInput::~SocketInput()
{
    Stop();
    Join();
    close(m_handle);
    close(m_client_handle);
}

void SocketInput::Task()
{
    while (m_state)
    {
        bool connected = true;
        socklen_t client_len;
        client_len = sizeof(m_client_addr);
        m_client_handle = accept(m_handle, (struct sockaddr *) &m_client_addr, &client_len);
        DBG_CHECK(m_client_handle < 0, connected = false, "Error: On connection accept");

        if (connected)
        {
            DBG_MSG("Client Connected");
        }

        while (connected)
        {
            Buffer* buf = GetEmptyBuffer();
            bzero(buf->GetData(), buf->GetMaxSize());

            size_t bytes_read = read(m_client_handle, buf->GetData(), buf->GetMaxSize());
            buf->SetSize(bytes_read);

            if (!memcmp(buf->GetData(), "Disconnect", buf->GetSize()))
            {
                DBG_MSG("Client Disconnected");
                connected = false;
                RecycleBuffer(buf);
                break;
            }

            ReceiveBuffer(buf);
        }
    }
}

Buffer* SocketInput::GetEmptyBuffer()
{
    if (m_device)
    {
        Buffer* buf = m_device->Output()->GetBuffer();
        if (buf)
        {
            return (buf);
        }
        else
        {
            return (InputPort::GetEmptyBuffer());
        }
    }
    else
    {
        return (InputPort::GetEmptyBuffer());
    }
}

VC_STATUS SocketInput::ReceiveBuffer(Buffer* buf)
{
    if (m_device)
    {
        if (m_device->Output()->PushBuffer(buf) != VC_SUCCESS)
        {
            return (InputPort::ReceiveBuffer(buf));
        }
    }
    else
    {
        return (InputPort::ReceiveBuffer(buf));
    }

    return (VC_SUCCESS);
}

SocketOutput::SocketOutput(std::string name, ADevice* device, const char* addr, int port) :
    OutputPort(name, device),
    m_handle(0),
    m_server_handle(0),
    m_connected(false)
{
    DBG_MSG("Enter");
    int err;

    m_handle = socket(2, SOCK_STREAM, IPPROTO_TCP);
    DBG_CHECK(m_handle < 0, return, "Error: Creating socket");

    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_port = htons(port);
    err = inet_pton(2, addr, &m_server_addr.sin_addr);
    DBG_CHECK(err <= 0, return, "Error(%d): Getting server address", err);

    err = connect(m_handle, (struct sockaddr*) &m_server_addr, sizeof(m_server_addr));
    DBG_CHECK(err < 0, delete this, "Error(%d): Connecting to server", err);

    m_connected = true;
    DBG_MSG("Connected to Server");

}

SocketOutput::~SocketOutput()
{
    size_t bytes = write(m_handle, "Disconnect", 10);
    DBG_CHECK(bytes != 10,, "Error: Unable send disconnect call");
    m_connected = false;
    DBG_MSG("Disconnected from Server");
    close(m_handle);
    close(m_server_handle);
}

Buffer* SocketOutput::GetBuffer()
{
    return (new Buffer());
}

VC_STATUS SocketOutput::PushBuffer(Buffer* buf)
{
    size_t bytes = write(m_handle, buf->GetData(), buf->GetSize());
    DBG_CHECK(bytes != buf->GetSize(), return (VC_FAILURE), "Error: Write data incomplete");
    return (VC_SUCCESS);
}

bool SocketOutput::IsConnected()
{
    return (m_connected);
}
