/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file basepipe.h is part of RPiCast project

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

#ifndef APIPE_H_
#define APIPE_H_

#include "utils.h"
#include "buffer.h"
#include "mutex.h"

class ADevice;
class BasePipe;

/**
 * An InputPort to a device that receives data.
 * Also takes care of the buffer management
 */
class InputPort
{

public:
    InputPort(std::string name, ADevice* device);
    virtual ~InputPort();

    virtual Buffer* GetFilledBuffer();
    virtual Buffer* GetEmptyBuffer();
    virtual bool IsBufferAvailable();
    virtual VC_STATUS RecycleBuffer(Buffer* buf);
    virtual VC_STATUS ReceiveBuffer(Buffer* buf);
    const char* c_str()
    {
        return (m_name.c_str());
    }

protected:
    ADevice* m_device;

private:
    std::list<Buffer*> m_buffers;
    std::list<Buffer*> m_processbuf;
    std::string m_name;
    Mutex m_queue_mutex;
    ConditionVariable m_queue_cv;

};

/**
 * OutputPort to a device that sends the data out
 */
class OutputPort
{
    friend class BasePipe;
public:
    OutputPort(std::string name, ADevice* device);
    virtual ~OutputPort();
    virtual VC_STATUS PushBuffer(Buffer* buf);
    virtual Buffer* GetBuffer();
    virtual VC_STATUS SetReceiver(InputPort* inport);
    virtual VC_STATUS ReturnBuffer(Buffer* buf);
    const char* c_str()
    {
        return (m_name.c_str());
    }

protected:
    ADevice* m_device;
    Mutex m_mutex;
    ConditionVariable m_cv;

private:
    std::string m_name;
    InputPort* m_receiver;
};

/**
 * Input parameters for the devices
 */
typedef struct
{
    int threshold; //Threshold audio input level
} InputParams;

/**
 * Output parameters from the devices
 */
typedef struct
{
    char** device_list; //Hardware devices on the system
} OutputParams;

/**
 * A pipe that maintains all the devices and their connections
 */
class BasePipe
{
public:
    BasePipe(std::string name);
    virtual ~BasePipe();

    /**
     * Initialize the pipe and its devices list
     */
    virtual VC_STATUS Initialize();

    /**
     * Uninitialize the pipe and devices list
     */
    virtual VC_STATUS Uninitialize();

    /**
     *  Query for the device type and add the device to m_devices to future reference
     */
    virtual VC_STATUS AddDevice(VC_DEVICETYPE dev, std::string name, const char* args = "");

    /**
     *  Remove the requested device from the pipes device list
     */
    virtual VC_STATUS RemoveDevice(VC_DEVICETYPE dev);

    /**
     * Find and return the device form the m_devices, if available
     * @param dev requested device type
     */
    virtual ADevice* FindDevice(VC_DEVICETYPE dev);

    /**
     * Send a start command to all the devices in the list
     */
    virtual VC_STATUS SendCommand(VC_CMD cmd);

    /**
     * Connects all the device in the sequence
     */
    virtual VC_STATUS Prepare();

    /**
     * Diaconnects all the devices in the list
     */
    virtual VC_STATUS Reset();

    /**
     * Query the pipe for available devices
     */
    virtual ADevice* GetDevice(VC_DEVICETYPE dev, std::string name, const char* args = "");

    /**
     * Connects the devices to their default ports i.e port 0
     * @param src source device
     * @param dst destination device
     */
    virtual VC_STATUS ConnectDevices(ADevice* src, ADevice* dst, int src_portno = 0, int dst_portno = 0);

    /**
     * Disconnects the devices from their default ports i.e port 0
     * @param src source device
     * @param dst destination device
     */
    virtual VC_STATUS DisconnectDevices(ADevice* src, ADevice* dst, int src_portno = 0, int dst_portno = 0);

    /**
     * Connect the specific ports irrespective of device's default
     * @param input port
     * @param output port
     */
    virtual VC_STATUS ConnectPorts(InputPort* input, OutputPort* output);

    /**
     * Disconnect the specific ports irrespective of device's default
     * @param input port
     * @param output port
     */
    virtual VC_STATUS DisconnectPorts(InputPort* input, OutputPort* output);

    const char* c_str()
    {
        return (m_name.c_str());
    }
private:
    std::string m_name;
    std::list<ADevice*> m_devices;
    std::map<VC_DEVICETYPE, ADevice*> m_devmap;
};

/**
 * An interface to all the devices
 */
class BaseDevice
{
public:
    BaseDevice()
    {
    }

    virtual ~BaseDevice()
    {
    }

    virtual VC_STATUS Initialize()=0;
    virtual VC_STATUS Uninitialize()=0;

    /**
     * Function to notify the device for any events
     */
    virtual VC_STATUS Notify(VC_EVENT* evt)=0;

    /**
     * Get the inputport of the device
     */
    virtual InputPort* Input(int portno)=0;

    /**
     * Get the output port of the device
     */
    virtual OutputPort* Output(int portno)=0;

    /**
     * Send a command to device. This method triggers the device to start or stop
     */
    virtual VC_STATUS SendCommand(VC_CMD cmd)=0;

    /**
     * Sets the required parameters for device
     */
    virtual VC_STATUS SetParameters(const InputParams* params)=0;

    /**
     * Gets the required parameters from device
     */
    virtual VC_STATUS GetParameters(OutputParams* params)=0;

    /**
     * Allocate a buffer to requesting member
     */
    virtual Buffer* AllocateBuffer(int portno)=0;

    /**
     * De-allocates the buffer allocated by the device
     */
    virtual VC_STATUS FreeBuffer(Buffer* buf)=0;

};

/**
 * A Base class for all the Devices in the application
 */
class ADevice: public BaseDevice
{
public:
    /**
     * ADevice Constructor
     */
    ADevice(std::string name, BasePipe* pipe = NULL) :
        m_name(name),
        m_mutex(),
        m_cv(m_mutex),
        m_pipe(pipe)
    {
    }

    /**
     * ADevice Destructor
     */
    virtual ~ADevice()
    {
    }

    /**
     * Initialize the device, calls the childs implementation
     */
    virtual VC_STATUS Initialize()
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Uninitialize the device, calls the childs implementation
     */
    virtual VC_STATUS Uninitialize()
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Function to notify the device for any events
     * TODO: update the api to notify different type of events, as needed
     */
    virtual VC_STATUS Notify(VC_EVENT* evt)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Get the inputport of the device
     */
    virtual InputPort* Input(int portno = 0)
    {
        return (NULL);
    }

    /**
     * Get the output port of the device
     */
    virtual OutputPort* Output(int portno = 0)
    {
        return (NULL);
    }

    /**
     * Send a command to device. This method triggers the device to start or stop
     */
    virtual VC_STATUS SendCommand(VC_CMD cmd)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Sets the required parameters for device
     */
    virtual VC_STATUS SetParameters(const InputParams* params)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Gets the required parameters from device
     */
    virtual VC_STATUS GetParameters(OutputParams* params)
    {
        return (VC_NOT_IMPLEMENTED);
    }

    /**
     * Allocate a buffer to requesting member
     */
    virtual Buffer* AllocateBuffer(int portno = 0)
    {
        return (new Buffer());
    }

    /**
     * De-allocates the buffer allocated by the device
     */
    virtual VC_STATUS FreeBuffer(Buffer* buf)
    {
        buf->Reset();
        delete buf;
        return (VC_SUCCESS);
    }

    /**
     * Name of the device for debugging purpose
     */
    virtual const char* c_str()
    {
        return (m_name.c_str());
    }

protected:
    std::string m_name;
    Mutex m_mutex;
    ConditionVariable m_cv;
    BasePipe* m_pipe;

};

#endif /* APIPE_H_ */
