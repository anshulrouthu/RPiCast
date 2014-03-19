/*
 * video_tunnel.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: anshul
 */

#include "utils.h"
#include "video_tunnel.h"

VideoTunnel::VideoTunnel(std::string name) :
    ADevice(name),
    m_input(NULL)
{
    m_video_decode = NULL;
    m_video_scheduler = NULL;
    m_video_render = NULL;
    m_clock = NULL;
    status = 0;
    data_len = 0;
    m_ready = false;
    bcm_host_init();
}

VideoTunnel::~VideoTunnel()
{
    DBG_ALL("Enter");

    ilclient_wait_for_event(m_video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, 100);
    ilclient_flush_tunnels(m_tunnel, 0);
    ilclient_disable_port_buffers(m_video_decode, 130, NULL, NULL, NULL);

    ilclient_disable_tunnel(m_tunnel);
    ilclient_disable_tunnel(m_tunnel + 1);
    ilclient_disable_tunnel(m_tunnel + 2);
    ilclient_teardown_tunnels(m_tunnel);

    ilclient_state_transition(m_list, OMX_StateIdle);
    ilclient_state_transition(m_list, OMX_StateLoaded);

    ilclient_cleanup_components(m_list);

    OMX_Deinit();

    ilclient_destroy(m_handle);
    delete m_input;
}

VC_STATUS VideoTunnel::Initialize()
{
    DBG_MSG("Enter");
    memset(m_list, 0, sizeof(m_list));
    memset(m_tunnel, 0, sizeof(m_tunnel));

    in = fopen("ScreenCap.out", "rb");

    m_input = new OMXInputPort(m_name + "_Input", this);

    if ((m_handle = ilclient_init()) == NULL)
    {
        return (VC_FAILURE);
    }

    if (OMX_Init() != OMX_ErrorNone)
    {
        ilclient_destroy(m_handle);
        return (VC_FAILURE);
    }

    // create video_decode
    if (ilclient_create_component(m_handle, &m_video_decode, "video_decode",
        (ILCLIENT_CREATE_FLAGS_T) (ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0)
        status = -14;
    m_list[0] = m_video_decode;

    // create video_render
    if (status == 0
        && ilclient_create_component(m_handle, &m_video_render, "video_render", ILCLIENT_DISABLE_ALL_PORTS) != 0)
        status = -14;
    m_list[1] = m_video_render;

    // create clock
    if (status == 0 && ilclient_create_component(m_handle, &m_clock, "clock", ILCLIENT_DISABLE_ALL_PORTS) != 0)
        status = -14;
    m_list[2] = m_clock;

    memset(&cstate, 0, sizeof(cstate));
    cstate.nSize = sizeof(cstate);
    cstate.nVersion.nVersion = OMX_VERSION;
    cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
    cstate.nWaitMask = 1;
    if (clock != NULL
        && OMX_SetParameter(ILC_GET_HANDLE(m_clock), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone)
        status = -13;

    // create video_scheduler
    if (status == 0
        && ilclient_create_component(m_handle, &m_video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0)
        status = -14;
    m_list[3] = m_video_scheduler;

    set_tunnel(m_tunnel, m_video_decode, 131, m_video_scheduler, 10);
    set_tunnel(m_tunnel + 1, m_video_scheduler, 11, m_video_render, 90);
    set_tunnel(m_tunnel + 2, m_clock, 80, m_video_scheduler, 12);

    // setup clock tunnel first
    if (status == 0 && ilclient_setup_tunnel(m_tunnel + 2, 0, 0) != 0)
        status = -15;
    else
        ilclient_change_component_state(m_clock, OMX_StateExecuting);

    if (status == 0)
        ilclient_change_component_state(m_video_decode, OMX_StateIdle);

    memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nVersion.nVersion = OMX_VERSION;
    format.nPortIndex = 130;
    format.eCompressionFormat = OMX_VIDEO_CodingMPEG2;

    return (VC_SUCCESS);
}

VC_STATUS VideoTunnel::Uninitialize()
{
    DBG_ALL("Enter");
    /*
     ilclient_disable_tunnel(m_tunnel);
     ilclient_disable_tunnel(m_tunnel+1);
     ilclient_disable_tunnel(m_tunnel+2);
     ilclient_teardown_tunnels(m_tunnel);

     ilclient_state_transition(m_list, OMX_StateIdle);
     ilclient_state_transition(m_list, OMX_StateLoaded);

     ilclient_cleanup_components(m_list);

     OMX_Deinit();

     ilclient_destroy(m_handle);
     */
    {
        AutoMutex automutex(&m_mutex);
        m_cv.Notify();
    }

    Join();

    DBG_ALL("Exit");
    return (VC_SUCCESS);
}

VC_STATUS VideoTunnel::Notify(VC_EVENT* evt)
{
    AutoMutex automutex(&m_mutex);
    m_cv.Notify();

    return (VC_SUCCESS);
}

InputPort* VideoTunnel::Input(int portno)
{
    return (m_input);
}

VC_STATUS VideoTunnel::SendCommand(VC_CMD cmd)
{
    switch (cmd)
    {
    case VC_CMD_START:
        Start();
        break;
    case VC_CMD_STOP:
        Stop();
        break;
    }
    return (VC_SUCCESS);
}

Buffer* VideoTunnel::AllocateBuffer(int portno)
{
    if(!m_ready)
    {
       AutoMutex automutex(&m_mutex);
       while(!m_ready)
       {
           m_cv.Wait();
       }
    }

    OMX_BUFFERHEADERTYPE* buf = ilclient_get_input_buffer(m_video_decode, 130, 1);
    return (new OMXBuffer(buf));
}

void VideoTunnel::Task()
{
    int err;
    int port_settings_changed = 0;
    int first_packet = 1;

    err = OMX_SetParameter(ILC_GET_HANDLE(m_video_decode), OMX_IndexParamVideoPortFormat, &format);
    DBG_CHECK(err, return, "Error(err): Unable to set parameter for video_decode");

    err = ilclient_enable_port_buffers(m_video_decode, 130, NULL, NULL, NULL);
    DBG_CHECK(err, return, "Error(err): Unable to enable video_decode port");

    ilclient_change_component_state(m_video_decode, OMX_StateExecuting);

    {
        AutoMutex automutex(&m_mutex);
        m_ready = true;
        m_cv.Notify();
    }

    while (m_state)
    {
        if (m_input->IsBufferAvailable())
        {
            OMXBuffer* b = dynamic_cast<OMXBuffer*>(m_input->GetFilledBuffer());

            size_t s = b->GetSize();

            if (port_settings_changed == 0
                && ((s > 0 && ilclient_remove_event(m_video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0)
                    || (s == 0
                        && ilclient_wait_for_event(m_video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1,
                            ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
            {
                port_settings_changed++;

                err = ilclient_setup_tunnel(m_tunnel, 0, 0);
                DBG_CHECK(err, break, "Error: Unable to setup tunnel");

                ilclient_change_component_state(m_video_scheduler, OMX_StateExecuting);

                err = ilclient_setup_tunnel(m_tunnel + 1, 0, 1000);
                DBG_CHECK(err, break, "Error: Unable to setup tunnel");

                ilclient_change_component_state(m_video_render, OMX_StateExecuting);
            }

            if(b->GetTag() == TAG_EOS)
            {
                DBG_MSG("Receiver EOS Tag");
                OMXBuffer* buf = dynamic_cast<OMXBuffer*>(AllocateBuffer());
                buf->m_omxbuf->nFilledLen = 0;
                buf->m_omxbuf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

                err = OMX_EmptyThisBuffer(ILC_GET_HANDLE(m_video_decode), buf->m_omxbuf);
                DBG_CHECK(err != OMX_ErrorNone, break, "Error: Unable to empty buffer");

                // wait for EOS from render
                ilclient_wait_for_event(m_video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, 100);

                // need to flush the renderer to allow video_decode to disable its input port
                ilclient_flush_tunnels(m_tunnel, 0);

                delete buf;
                delete b;
                continue;
            }

            if (first_packet)
            {
                b->m_omxbuf->nFlags = OMX_BUFFERFLAG_STARTTIME;
                first_packet = 0;
            }
            else
            {
                b->m_omxbuf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
            }

            b->m_omxbuf->nOffset = 0;
            err = OMX_EmptyThisBuffer(ILC_GET_HANDLE(m_video_decode), b->m_omxbuf);
            DBG_CHECK(err != OMX_ErrorNone, break, "Error: Unable to empty buffer");

            delete b;
        }
        else
        {
            AutoMutex automutex(&m_mutex);
            while (!m_input->IsBufferAvailable() && m_state)
            {
                m_cv.Wait();
            }
        }
    }

    OMX_BUFFERHEADERTYPE *buf;
    buf = dynamic_cast<OMXBuffer*>(AllocateBuffer())->m_omxbuf;
    buf->nFilledLen = 0;
    buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

    if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(m_video_decode), buf) != OMX_ErrorNone)
    {
        status = -20;
    }

    // wait for EOS from render
    ilclient_wait_for_event(m_video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, 100);

    // need to flush the renderer to allow video_decode to disable its input port
    ilclient_flush_tunnels(m_tunnel, 0);

    ilclient_disable_port_buffers(m_video_decode, 130, NULL, NULL, NULL);
}

OMXInputPort::OMXInputPort(std::string name, ADevice* device) :
    InputPort(name, device),
    m_mutex(),
    m_cv(m_mutex)
{
}

Buffer* OMXInputPort::GetEmptyBuffer()
{
    return (m_device->AllocateBuffer());
}

OMXBuffer::OMXBuffer(OMX_BUFFERHEADERTYPE* buf)
{
    m_omxbuf = buf;
    m_tag = TAG_NONE;
}

size_t OMXBuffer::GetMaxSize()
{
    return (m_omxbuf->nAllocLen);
}

size_t OMXBuffer::GetSize()
{
    return (m_omxbuf->nFilledLen);
}

VC_STATUS OMXBuffer::SetSize(size_t size)
{
    m_omxbuf->nFilledLen = size;
    return (VC_SUCCESS);
}

void* OMXBuffer::GetData()
{
    return (m_omxbuf->pBuffer);
}

VC_STATUS OMXBuffer::WriteData(void* data, size_t size)
{
    DBG_CHECK(!memcpy(m_omxbuf->pBuffer, data, size), return (VC_FAILURE), "Error writing data to buffer");
    m_omxbuf->nFilledLen += size;
    return (VC_SUCCESS);
}
