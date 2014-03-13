/*
 * video_tunnel.h
 *
 *  Created on: Mar 13, 2014
 *      Author: anshul
 */

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
    virtual ~OMXInputPort(){}

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
    virtual ~OMXBuffer(){}

    virtual size_t GetMaxSize();
    virtual size_t GetSize();
    virtual void* GetData();
    virtual VC_STATUS SetSize(size_t size);
private:
    OMX_BUFFERHEADERTYPE* m_omxbuf;
};

#endif /* VIDEO_TUNNEL_H_ */
