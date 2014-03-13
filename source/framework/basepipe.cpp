/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

#include "basepipe.h"

/**
 * Number of input buffers
 */
#define NUM_OF_BUFFERS 16

/**
 * APipe constructor
 * @param name to identify the pipe
 */
BasePipe::BasePipe(std::string name) :
    m_name(name)
{
}

/**
 * Returns the queried device based on the VC_DEVICETYPE
 * @param[in] devtype type of device requested
 * @param[in] name of the device to be names for identification
 * @return device instance of the device available based on type
 */
ADevice* BasePipe::GetDevice(VC_DEVICETYPE devtype, std::string name, const char* args)
{
    DBG_TRACE("Enter");
    return (NULL);
}

/**
 * Connects the devices to their default ports i.e port 0
 * @param src source device
 * @param dst destination device
 */
VC_STATUS BasePipe::ConnectDevices(ADevice* src, ADevice* dst, int src_portno, int dst_portno)
{
    DBG_TRACE("Enter");
    DBG_CHECK(!src || !dst, return (VC_FAILURE), "Error: Null parameters");
    return (ConnectPorts(dst->Input(dst_portno), src->Output(src_portno)));
}

/**
 * Disconnects the devices from their default ports i.e port 0
 * @param src source device
 * @param dst destination device
 */
VC_STATUS BasePipe::DisconnectDevices(ADevice* src, ADevice* dst, int src_portno, int dst_portno)
{
    DBG_TRACE("Enter");
    DBG_CHECK(!src || !dst, return (VC_FAILURE), "Error: Null parameters");
    return (DisconnectPorts(dst->Input(dst_portno), src->Output(src_portno)));
}

/**
 * Connect the specific ports irrespective of device's default
 * @param input port
 * @param output port
 */
VC_STATUS BasePipe::ConnectPorts(InputPort* input, OutputPort* output)
{
    DBG_TRACE("Enter");
    DBG_CHECK(!input || !output, return (VC_FAILURE), "Error: Null parameters");
    return (output->SetReceiver(input));
}

/**
 * Disconnect the specific ports irrespective of device's default
 * @param input port
 * @param output port
 */
VC_STATUS BasePipe::DisconnectPorts(InputPort* input, OutputPort* output)
{
    DBG_TRACE("Enter");
    DBG_CHECK(!input || !output, return (VC_FAILURE), "Error: Null parameters");
    DBG_CHECK(output->m_receiver != input, return (VC_FAILURE), "Error: Invalid ports");
    return (output->SetReceiver(NULL));
}

/**
 * Inputport constructor
 * @param name to identify this input port
 * @param device this input port belongs to
 */
InputPort::InputPort(std::string name, ADevice* device) :
    m_device(device),
    m_name(name),
    m_queue_cv(m_queue_mutex)
{
    DBG_TRACE("Enter");
    for (int i = 0; i < NUM_OF_BUFFERS; i++)
    {
        Buffer* buf = new Buffer();
        m_buffers.push_back(buf);
    }
}

InputPort::~InputPort()
{
    for (std::list<Buffer*>::iterator it = m_buffers.begin(); it != m_buffers.end(); it++)
    {
        delete *it;
    }

    m_buffers.clear();
}
/**
 * Returns the filled buffer from the received buffer queue
 * @return buf to be processed by device
 */
Buffer* InputPort::GetFilledBuffer()
{
    DBG_TRACE("Enter");
    AutoMutex automutex(&m_queue_mutex);
    DBG_CHECK(m_processbuf.size() == 0, return (NULL), "No buffers available to be processed");

    Buffer* buf = m_processbuf.front();
    m_processbuf.pop_front();

    return (buf);
}

/**
 * Returns an empty buffer. from the pre-allocated buffer pool to be filled with data
 * @return buf, data to be filled in
 */
Buffer* InputPort::GetEmptyBuffer()
{
    DBG_TRACE("Enter");
    AutoMutex automutex(&m_queue_mutex);
    while (m_buffers.size() == 0)
    {
        //this should not happen or we starve for buffer
        DBG_ERR("Waiting for buffers");
        m_queue_cv.Wait();
    }

    Buffer* buf = m_buffers.front();
    m_buffers.pop_front();

    return (buf);
}

/**
 * Recycles the buffer and stores in the empty buffer pool
 * @param buf to be recycled
 */
VC_STATUS InputPort::RecycleBuffer(Buffer* buf)
{
    DBG_TRACE("Enter");
    AutoMutex automutex(&m_queue_mutex);
    buf->Reset();
    m_buffers.push_back(buf);
    m_queue_cv.Notify();
    return (VC_SUCCESS);
}

/**
 * Receives buffer from the connected port
 * @param buf received buffer
 */
VC_STATUS InputPort::ReceiveBuffer(Buffer* buf)
{
    DBG_TRACE("Enter");
    {
        AutoMutex automutex(&m_queue_mutex);
        m_processbuf.push_back(buf);
    }

    /*
     * This mutex should be released before Notify call, some device
     * block this call, and access the input buffer e.g filesink device
     * TODO: Improve this notification mechanism
     */
    if (m_device)
    {
        m_device->Notify(NULL);
    }

    return (VC_SUCCESS);
}

/**
 * Checks if a buffer is available for processing
 * @return true/false
 */
bool InputPort::IsBufferAvailable()
{
    DBG_TRACE("Enter");
    AutoMutex automutex(&m_queue_mutex);
    return (m_processbuf.size());
}

/**
 * Outputport constructor
 * @param name to identify this output port
 * @param device this output port belongs to
 */
OutputPort::OutputPort(std::string name, ADevice* device) :
    m_device(device),
    m_name(name),
    m_receiver(NULL)
{
}

/**
 * Sets the receiver of the output port
 * @param inport the input port to connect to
 */
VC_STATUS OutputPort::SetReceiver(InputPort* inport)
{
    DBG_TRACE("Enter");
    if ((m_receiver && !inport) || (!m_receiver && inport))
    {
        m_receiver = inport;
        return (VC_SUCCESS);
    }
    DBG_ERR("Error: Cannot connect receiver");
    return (VC_FAILURE);
}

/**
 * Pushes the buffer to is receiver (connected input port)
 * @param buf a buffer to be pushed
 */
VC_STATUS OutputPort::PushBuffer(Buffer* buf)
{
    DBG_TRACE("Enter %d",buf->GetSize());
    if (m_receiver)
    {
        return (m_receiver->ReceiveBuffer(buf));
    }
    return (VC_FAILURE);
}

/**
 * Gets the buffer from the connected input port for data to be filled
 * @return buf
 */
Buffer* OutputPort::GetBuffer()
{
    DBG_TRACE("Enter");
    if (m_receiver)
    {
        return (m_receiver->GetEmptyBuffer());
    }

    return (NULL);
}

/**
 * return the buffer to input port if the buffer is not used
 * @param buf
 */
VC_STATUS OutputPort::ReturnBuffer(Buffer* buf)
{
    return (m_receiver->RecycleBuffer(buf));
}
