/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file video_tunnel.h is part of RPiCast project

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

#ifndef VIDEO_TUNNEL_H_
#define VIDEO_TUNNEL_H_

#include "utils.h"
#include "bcm_host.h"
#include "ilclient.h"
#include "basepipe.h"
#include "worker.h"

class OMXInputPort;
class OMXBuffer;

class VideoTunnel: public ADevice, public WorkerThread
{
public:
    VideoTunnel(std::string name);
    virtual ~VideoTunnel();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Uninitialize();
    virtual InputPort* Input(int portno);
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual VC_STATUS SendCommand(VC_CMD cmd);
    virtual Buffer* AllocateBuffer(int portno = 0);

private:

    virtual void Task();
    InputPort* m_input;
    COMPONENT_T* m_video_decode;
    COMPONENT_T* m_video_scheduler;
    COMPONENT_T* m_video_render;
    COMPONENT_T* m_clock;
    TUNNEL_T m_tunnel[4];
    ILCLIENT_T* m_handle;
    COMPONENT_T* m_list[5];

    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
    FILE *in;
    int status;
    size_t data_len;
    bool m_ready;
};

class OMXInputPort: public InputPort
{

public:
    OMXInputPort(std::string name, ADevice* device);
    virtual ~OMXInputPort()
    {
    }

    virtual Buffer* GetEmptyBuffer();
private:
    Mutex m_mutex;
    ConditionVariable m_cv;
};

class OMXBuffer: public Buffer
{
    friend class VideoTunnel;
public:
    OMXBuffer(OMX_BUFFERHEADERTYPE* buf);
    virtual ~OMXBuffer()
    {
    }

    virtual size_t GetMaxSize();
    virtual size_t GetSize();
    virtual void* GetData();
    virtual VC_STATUS WriteData(void* buf, size_t size);
    virtual VC_STATUS SetSize(size_t size);
    OMX_BUFFERHEADERTYPE* m_omxbuf;
private:

};

#endif /* VIDEO_TUNNEL_H_ */
