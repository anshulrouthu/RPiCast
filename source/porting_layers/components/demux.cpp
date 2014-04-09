/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file demux.cpp is part of RPiCast project

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
 *********************************************************************/

#include "demux.h"

DemuxDevice::DemuxDevice(std::string name, BasePipe* pipe) :
    ADevice(name, pipe),
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

    m_h264filter_ctx = av_bitstream_filter_init("h264_mp4toannexb");
    DBG_CHECK(!m_h264filter_ctx,, "Error(%d): Unable to create H264 bit stream filter", m_h264filter_ctx);

    m_input = new CustomIOPort(m_name + "Input", this);

    for (int i = 0; i < 2; i++)
    {
        m_output[i] = new OutputPort(m_name + "Output", this);
    }

    return (VC_SUCCESS);
}

VC_STATUS DemuxDevice::Uninitialize()
{
    DBG_MSG("Enter");

    m_input->CloseInput();
    Join();

    delete m_input;

    for (std::map<int, OutputPort*>::iterator it = m_output.begin(); it != m_output.end(); it++)
    {
        delete it->second;
        it->second = NULL;
    }

    if (m_h264filter_ctx)
    {
        av_free(m_h264filter_ctx);
    }

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
    {
        AutoMutex automutex(&m_mutex);
        Stop();
        break;
    }
    default:
        break;
    }
    return (VC_SUCCESS);
}

void DemuxDevice::Task()
{
    AVPacket pkt;
    av_init_packet(&pkt);
    int err;
    m_preroll = true;

    while (m_state)
    {
        if (Input()->IsBufferAvailable() || !m_preroll)
        {
            if (m_preroll)
            {
                m_ctx = avformat_alloc_context();
                m_input->OpenInput(m_ctx);

                avformat_open_input(&m_ctx, "", NULL, NULL);
                //m_ctx->max_analyze_duration = AV_TIME_BASE / 10; /* analyze 1/10 of a second of data */
                //avformat_find_stream_info(m_ctx, NULL);
                m_preroll = false;
            }

            pkt.data = NULL;
            pkt.size = 0;
            err = av_read_frame(m_ctx, &pkt);
            if (err == (int) AVERROR_EOF && m_state)
            {
                DBG_MSG("Received EOS Tag");
                Buffer* buf = Output(0)->GetBuffer();
                buf->SetTag(TAG_EOS);
                Output(0)->PushBuffer(buf);
                m_preroll = true;
                m_input->CloseInput();

                continue;

            }
            if (m_ctx->streams[pkt.stream_index]->codec->codec_id == AV_CODEC_ID_H264 && 0)
            {
                AVPacket new_pkt = pkt;
                //av_init_packet(&new_pkt);
                int result = av_bitstream_filter_filter(m_h264filter_ctx, m_ctx->streams[pkt.stream_index]->codec, NULL, &new_pkt.data,
                    &new_pkt.size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);

                /* if bitstream filter failed, just print a warning because it may be OK for
                 * some cases, such as BD AVC video */
                if (result < 0)
                {
                    DBG_ERR("H264 bitstream filter failed.%d", result);
                }
                else
                {
                    pkt = new_pkt;
                }

                //av_free_packet(&new_pkt);
            }

            if (err == 0 && m_state)
            {
                if (m_ctx->streams[pkt.stream_index]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    AutoMutex automutex(&m_mutex);
                    int offset = 0;
                    while ((offset < pkt.size) && m_state)
                    {
                        Buffer* buf = Output(0)->GetBuffer();
                        int write_bytes = MIN((pkt.size - offset),buf->GetMaxSize());
                        buf->WriteData(pkt.data + offset, write_bytes);
                        offset += write_bytes;

                        Output(0)->PushBuffer(buf);
                    }
                }
            }
            av_free_packet(&pkt);
        }
        else
        {
            AutoMutex automutex(&m_input->m_wait_mutex);
            while (!Input()->IsBufferAvailable() && m_state)
            {
                m_input->m_wait_cv.Wait();
            }
        }
    }
}

CustomIOPort::CustomIOPort(std::string name, ADevice* device) :
    InputPort(name, device),
    m_ctx(NULL),
    m_buffer(NULL),
    m_rd_offset(0),
    m_wait_mutex(),
    m_wait_cv(m_wait_mutex),
    m_initialized(false)
{
}

CustomIOPort::~CustomIOPort()
{
}

VC_STATUS CustomIOPort::OpenInput(AVFormatContext* ctx)
{
    AutoMutex automutex(&m_wait_mutex);
    const int buffer_size = 32 * 1024;
    ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    ctx->pb = avio_alloc_context((unsigned char*) av_malloc(buffer_size), buffer_size, 0, this, read_cb, NULL, seek_cb);
    ctx->pb->seekable = false;
    //ctx->probesize = buffer_size/4;

    m_ctx = ctx;
    m_initialized = true;

    return (VC_SUCCESS);
}

VC_STATUS CustomIOPort::CloseInput()
{
    AutoMutex automutex(&m_wait_mutex);

    if (m_ctx && m_ctx->iformat)
    {
        av_free(m_ctx->pb->buffer);
        av_free(m_ctx->pb);
        avformat_close_input(&m_ctx);
    }

    m_initialized = false;
    m_wait_cv.Notify();

    return (VC_SUCCESS);
}

int CustomIOPort::read(uint8_t *buf, int buf_size)
{
    int read_size = AVERROR_EOF;

    if (!IsBufferAvailable() && !m_buffer)
    {
        AutoMutex automutex(&m_wait_mutex);
        while (!IsBufferAvailable() && m_initialized)
        {
            m_wait_cv.Wait();
        }

        /* We do not want to process any buffers while we are exiting */
        if (!m_initialized)
        {
            return (read_size);
        }
    }

    if (!m_buffer)
    {
        m_buffer = GetFilledBuffer();
        if (m_buffer->GetTag() != TAG_NONE)
        {
            DBG_MSG("Received EOS");
            RecycleBuffer(m_buffer);
            m_buffer = NULL;
            return (read_size);
        }
    }

    read_size = MIN((m_buffer->GetSize() - m_rd_offset), buf_size);
    memcpy(buf, m_buffer->GetData() + m_rd_offset, read_size);
    m_rd_offset += read_size;

    if (m_rd_offset >= m_buffer->GetSize())
    {
        RecycleBuffer(m_buffer);
        m_buffer = NULL;
        m_rd_offset = 0;
    }

    return (read_size);
}

int CustomIOPort::read_cb(void *opaque, uint8_t *buf, int buf_size)
{
    CustomIOPort* self = static_cast<CustomIOPort*>(opaque);
    return (self->read(buf, buf_size));
}

int64_t CustomIOPort::seek_cb(void *opaque, int64_t offset, int whence)
{
    return (-1);
}
