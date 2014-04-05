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
#define NUM_OF_BUFFERS 64

/**
 * Pipe constructor
 * @param name to identify the pipe
 */
BasePipe::BasePipe(std::string name) :
    m_name(name)
{
}

/**
 * Pipe Destructor
 */
BasePipe::~BasePipe()
{
    for(std::list<ADevice*>::iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        delete (*it);
        (*it) = NULL;
    }

    m_devices.clear();
    m_devmap.clear();
}

/**
 * Initialize the pipe
 */
VC_STATUS BasePipe::Initialize()
{
    VC_STATUS err;
    for(std::list<ADevice*>::iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        err = (*it)->Initialize();
        DBG_CHECK(err != VC_SUCCESS, return (VC_FAILURE),"Error: Unable to initialize device %s", (*it)->c_str());
    }
    return (VC_SUCCESS);
}

/**
 * Uninitialize the pipe
 */
VC_STATUS BasePipe::Uninitialize()
{
    VC_STATUS err;
    for(std::list<ADevice*>::iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        err = (*it)->Uninitialize();
        DBG_CHECK(err != VC_SUCCESS, return (VC_FAILURE),"Error: Unable to uninitialize device %s", (*it)->c_str());
    }
    return (VC_SUCCESS);
}

/**
 * Add the requested devices to the pipe's device list
 * @param devtype device type
 * @param name device name
 * @param args arguments to device
 */
VC_STATUS BasePipe::AddDevice(VC_DEVICETYPE devtype, std::string name, const char* args)
{
    ADevice* device = GetDevice(devtype, name, args);
    m_devices.push_back(device);
    m_devmap[devtype] = device;

    return (VC_SUCCESS);
}

/**
 * Remove the requested device list from pipe
 * @param dev detice type
 */
VC_STATUS BasePipe::RemoveDevice(VC_DEVICETYPE dev)
{
    for(std::list<ADevice*>::iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        if(m_devmap[dev] == (*it))
        {
             delete (*it);
             (*it) = NULL;
             m_devices.erase(it);
             m_devmap.erase(dev);
             return (VC_SUCCESS);
        }
    }

    return (VC_FAILURE);
}

/**
 * Send command to all the devices
 * @param cmd command to send to devices
 */
VC_STATUS BasePipe::SendCommand(VC_CMD cmd)
{
    VC_STATUS err;
    for(std::list<ADevice*>::iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        err = (*it)->SendCommand(cmd);
        DBG_CHECK(err != VC_SUCCESS, return (VC_FAILURE), "Error: Unable to send command to %s", (*it)->c_str());
    }

    return (VC_SUCCESS);
}

/**
 * Connect all the devices in the list
 */
VC_STATUS BasePipe::Prepare()
{
    VC_STATUS err;
    for(std::list<ADevice*>::iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        if(it != m_devices.begin())
        {
            std::list<ADevice*>::iterator prev = it;
            --prev;

            err = ConnectDevices((*prev), (*it));
            DBG_CHECK(err != VC_SUCCESS, return (VC_FAILURE), "Error: Unable to connect devices %s %s", (*prev)->c_str(), (*it)->c_str());
        }

    }

    return (VC_SUCCESS);
}

/**
 * Disconnect all the devices in the list
 */
VC_STATUS BasePipe::Reset()
{
    VC_STATUS err;
    for(std::list<ADevice*>::iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        if(it != m_devices.begin())
        {
            std::list<ADevice*>::iterator prev = it;
            --prev;

            err = DisconnectDevices((*prev), (*it));
            DBG_CHECK(err != VC_SUCCESS, return (VC_FAILURE), "Error: Unable to connect devices %s %s", (*prev)->c_str(), (*it)->c_str());
        }
    }

    return (VC_SUCCESS);
}
/**
 * Find and return the requested device
 */
ADevice* BasePipe::FindDevice(VC_DEVICETYPE dev)
{
    for(std::list<ADevice*>::iterator it = m_devices.begin(); it != m_devices.end(); it++)
    {
        if(m_devmap[dev] == (*it))
        {
            return (*it);
        }
    }

    return (NULL);
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
    DBG_MSG("Connecting devices %s ----> %s", src ? src->c_str() : "NULL", dst ? dst->c_str() : "NULL");
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
    DBG_MSG("Disconnecting devices %s --X--> %s", src ? src->c_str() : "NULL", dst ? dst->c_str() : "NULL");
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
    DBG_MSG("Enter");
    AutoMutex automutex(&m_queue_mutex);
    for (int i = 0; i < NUM_OF_BUFFERS; i++)
    {
        Buffer* buf = new Buffer();
        m_buffers.push_back(buf);
    }
}

InputPort::~InputPort()
{
    AutoMutex automutex(&m_queue_mutex);
    for (std::list<Buffer*>::iterator it = m_buffers.begin(); it != m_buffers.end(); it++)
    {
        delete *it;
    }

    for (std::list<Buffer*>::iterator it = m_processbuf.begin(); it != m_processbuf.end(); it++)
    {
        delete *it;
    }

    m_buffers.clear();
    m_processbuf.clear();
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
    m_mutex(),
    m_cv(m_mutex),
    m_name(name),
    m_receiver(NULL)
{
    DBG_MSG("Enter");
}

OutputPort::~OutputPort()
{
    {
        AutoMutex automutex(&m_mutex);
        m_cv.Notify();
    }
}
/**
 * Sets the receiver of the output port
 * @param inport the input port to connect to
 */
VC_STATUS OutputPort::SetReceiver(InputPort* inport)
{
    DBG_TRACE("Enter");
    AutoMutex automutex(&m_mutex);
    if ((m_receiver && !inport) || (!m_receiver && inport))
    {
        m_receiver = inport;
        m_cv.Notify();
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
    DBG_TRACE("Enter");
    if (m_receiver)
    {
        return (m_receiver->ReceiveBuffer(buf));
    }
    DBG_ERR("Error: No receiver set");
    return (VC_FAILURE);
}

/**
 * Gets the buffer from the connected input port for data to be filled
 * @return buf
 */
Buffer* OutputPort::GetBuffer()
{
    DBG_TRACE("Enter");
    if (!m_receiver)
    {
        AutoMutex automutex(&m_mutex);
        if (!m_receiver)
        {
            DBG_ERR("Waiting for receiver");
            m_cv.Wait();
        }
    }

    return (m_receiver->GetEmptyBuffer());
}

/**
 * return the buffer to input port if the buffer is not used
 * @param buf
 */
VC_STATUS OutputPort::ReturnBuffer(Buffer* buf)
{
    return (m_receiver->RecycleBuffer(buf));
}
