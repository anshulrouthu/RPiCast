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
    m_server_addr(addr),
    m_device_ready(false)
{
    DBG_MSG("Enter");
}

VC_STATUS SocketOutDevice::SendCommand(VC_CMD cmd)
{
    DBG_MSG("Enter");

    AutoMutex automutex(&m_mutex);
    switch (cmd)
    {
    case VC_CMD_START:
        m_socket_output = new SocketOutput(m_name + "_Input", this, m_server_addr);
        DBG_CHECK(!m_socket_output, return (VC_FAILURE), "Error: Creating socket out device");
        break;
    case VC_CMD_STOP:
        delete m_socket_output;
        m_socket_output = NULL;
        break;
    }
    return (VC_SUCCESS);
}

VC_STATUS SocketOutDevice::Notify(VC_EVENT* evt)
{
    AutoMutex automutex(&m_mutex);

    while (Input()->IsBufferAvailable() && m_socket_output)
    {
        Buffer* buf = Input()->GetFilledBuffer();
        m_socket_output->PushBuffer(buf);
        Input()->RecycleBuffer(buf);
    }
    return (VC_SUCCESS);
}

SocketInput::SocketInput(std::string name, ADevice* device, const char* addr, int port) :
    InputPort(name, device),
    m_handle(0)
{
    DBG_MSG("Enter");
    m_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    DBG_CHECK(m_handle < 0, return, "Error: Creating socket");

    bzero((char *) &m_server_addr, sizeof(m_server_addr));
    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_addr.s_addr = INADDR_ANY;
    m_server_addr.sin_port = htons(port);

    int err = bind(m_handle, (struct sockaddr *) &m_server_addr, sizeof(m_server_addr));
    DBG_CHECK(err < 0, return, "Error(%d): Unable to bind socket", err);

    Start();
}

SocketInput::~SocketInput()
{
    Stop();
    shutdown(m_handle,SHUT_RDWR);
    close(m_handle);
    Join();
}

void SocketInput::Task()
{
    while (m_state)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len;
        client_len = sizeof(client_addr);

        Buffer* buf = GetEmptyBuffer();
        bzero(buf->GetData(), buf->GetMaxSize());

        size_t bytes_read = recvfrom(m_handle, buf->GetData(), buf->GetMaxSize(), 0, (struct sockaddr *) &client_addr, &client_len);
        buf->SetSize(bytes_read);

        if (!memcmp(buf->GetData(), "Disconnect", buf->GetSize()))
        {
            DBG_MSG("Client Disconnected");
            buf->SetTag(TAG_EOS);
            ReceiveBuffer(buf);
            continue;
        }

        ReceiveBuffer(buf);
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
    m_handle(0)
{
    DBG_MSG("Connecting to:%s", addr);
    int err;

    m_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    DBG_CHECK(m_handle < 0, return, "Error: Creating socket");

    memset((char *) &m_client_addr, 0, sizeof(m_client_addr));
    m_client_addr.sin_family = AF_INET;
    m_client_addr.sin_addr.s_addr = htonl(INADDR_ANY );
    m_client_addr.sin_port = htons(0);

    if (bind(m_handle, (struct sockaddr *) &m_client_addr, sizeof(m_client_addr)) < 0)
    {
        perror("bind failed");
    }

    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_port = htons(port);
    err = inet_aton(addr, &m_server_addr.sin_addr);

    DBG_CHECK(err <= 0, return, "Error(%d): Getting server address", err);
}

SocketOutput::~SocketOutput()
{
    size_t bytes = sendto(m_handle, "Disconnect", 10, 0, (struct sockaddr *) &m_server_addr, sizeof(m_server_addr));
    DBG_CHECK(bytes != 10,, "Error: Unable send disconnect call");
    close(m_handle);
}

Buffer* SocketOutput::GetBuffer()
{
    return (new Buffer());
}

VC_STATUS SocketOutput::PushBuffer(Buffer* buf)
{
    int read_size = 0;
    int offset = 0;

    while (offset < buf->GetSize())
    {
        read_size = MIN(buf->GetSize() - offset, UDP_PACKET_MAXSIZE);
        size_t bytes = sendto(m_handle, buf->GetData() + offset, read_size, 0, (struct sockaddr *) &m_server_addr, sizeof(m_server_addr));
        DBG_CHECK(bytes != read_size, return (VC_FAILURE), "Error(%d): Data sent incomplete %d", bytes, read_size);
        offset += read_size;
    }
    return (VC_SUCCESS);
}
