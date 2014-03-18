/*
 * demux.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: anshul
 */

#include "demux.h"

DemuxDevice::DemuxDevice(std::string name, BasePipe* pipe):
    ADevice(name,pipe),
    m_input(NULL),
    m_ctx(NULL)
{
    DBG_MSG("Enter");
}

DemuxDevice::~DemuxDevice()
{
    DBG_MSG("Enter");
}

VC_STATUS DemuxDevice::Initialize()
{
    DBG_MSG("Enter");

    av_register_all();

    m_ctx = avformat_alloc_context();
    m_input = new CustomIOPort(m_ctx, m_name+"Input", this);

    for(int i=0; i < 2; i++)
    {
        m_output[i] = new OutputPort(m_name+"Output", this);
    }

    return (VC_SUCCESS);
}

VC_STATUS DemuxDevice::Uninitialize()
{
    DBG_MSG("Enter");

    delete m_input;

    for(std::map<int,OutputPort*>::iterator it = m_output.begin(); it != m_output.end(); it++)
    {
        delete (*it).second;
        (*it).second = NULL;
    }

    if (m_ctx && m_ctx->iformat)
    {
        av_free(m_ctx->pb->buffer);
        av_free(m_ctx->pb);
        avformat_close_input(&m_ctx);
    }

    Join();
    return (VC_SUCCESS);
}

InputPort* DemuxDevice::Input(int portno)
{
    return (m_input);
}

OutputPort* DemuxDevice::Output(int portno)
{
    return (m_output[portno]);
}

VC_STATUS DemuxDevice::Notify(VC_EVENT* evt)
{
    /* wake up the customIO which is waiting for buffers */
    AutoMutex automutex(&m_input->m_wait_mutex);
    m_input->m_wait_cv.Notify();

    return (VC_SUCCESS);
}

VC_STATUS DemuxDevice::SendCommand(VC_CMD cmd)
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

void DemuxDevice::Task()
{
    AVPacket pkt;
    av_init_packet(&pkt);
    int err;

    avformat_open_input(&m_ctx, "", NULL, NULL);

    m_ctx->max_analyze_duration = AV_TIME_BASE / 2; /* analyze 1/10 of a second of data */
    avformat_find_stream_info(m_ctx, NULL);

    while(m_state)
    {
        if(Input()->IsBufferAvailable())
        {
            pkt.data = NULL;
            pkt.size = 0;
            err = av_read_frame(m_ctx, &pkt);

            if(err == 0 && m_state)
            {
                if(m_ctx->streams[pkt.stream_index]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    Buffer* buf = Output(0)->GetBuffer();
                    buf->WriteData(pkt.data,pkt.size);
                    Output(0)->PushBuffer(buf);
                }
            }
            av_free_packet(&pkt);
        }
        /* wait condition is implemented in customIO */
    }
}

CustomIOPort::CustomIOPort(AVFormatContext* ctx, std::string name, ADevice* device):
    InputPort(name, device),
    m_ctx(ctx),
    m_buffer(NULL),
    m_rd_offset(0),
    m_wait_mutex(),
    m_wait_cv(m_wait_mutex)
{
    const int buffer_size = 1024*1024;
    ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    ctx->pb = avio_alloc_context((unsigned char*)av_malloc(buffer_size), buffer_size, 0, this,read_cb, NULL, seek_cb);
    ctx->pb->seekable = false;
    //ctx->probesize = buffer_size/4;

    m_ctx = ctx;
    m_initialized = true;
}

CustomIOPort::~CustomIOPort()
{
    m_initialized = false;

    {
        AutoMutex automutex(&m_wait_mutex);
        m_wait_cv.Notify();
    }
}

int CustomIOPort::read(uint8_t *buf, int buf_size)
{
    int read_size = 0;

    if (IsBufferAvailable() || m_buffer)
    {
        if(!m_buffer)
        {
            m_buffer = GetFilledBuffer();
        }

        if (m_buffer->GetSize())
        {
            read_size = MIN((m_buffer->GetSize()), buf_size);
            memcpy(buf, m_buffer->GetData() + m_rd_offset, read_size);
            m_rd_offset += read_size;
        }

        if (m_rd_offset >= m_buffer->GetSize())
        {
            RecycleBuffer(m_buffer);
            m_buffer = NULL;
            m_rd_offset = 0;
        }
    }
    else
    {
        AutoMutex automutex(&m_wait_mutex);
        while(!IsBufferAvailable() && m_initialized)
        {
            m_wait_cv.Wait();
        }
    }
    return (read_size);
}

int CustomIOPort::read_cb(void *opaque, uint8_t *buf, int buf_size)
{
    CustomIOPort* self = static_cast<CustomIOPort*>(opaque);
    return (self->read(buf,buf_size));
}

int64_t CustomIOPort::seek_cb(void *opaque, int64_t offset, int whence)
{
    return (-1);
}
