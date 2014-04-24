/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file video_encoder.cpp is part of RPiCast project

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

#include "video_encoder.h"

VideoEncoder::VideoEncoder(std::string name) :
    ADevice(name)
{
}

VideoEncoder::~VideoEncoder()
{
    DBG_MSG("Enter");
}

VC_STATUS VideoEncoder::Initialize()
{
    DBG_MSG("Enter");
    AVCodec* codec;

    m_input = new InputPort("VidEncoder_Input", this);
    m_output = new OutputCustomIO("VidEncoder_Output", this);

    m_h264filter_ctx = av_bitstream_filter_init("h264_mp4toannexb");

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    DBG_CHECK(!codec, return (VC_FAILURE), "Error: Encoder codec not found");

    m_fmtCtx = avformat_alloc_context();
    m_fmtCtx->oformat = av_guess_format(NULL, "out.m2ts", NULL);
    m_vidstream = avformat_new_stream(m_fmtCtx, codec);

    m_encodeCtx = m_vidstream->codec;
    DBG_CHECK(!m_encodeCtx, return (VC_FAILURE), "Error: Unable to allocate encoder codec context");

    av_opt_set(m_encodeCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(m_encodeCtx->priv_data, "tune", "zerolatency", 0);
    av_opt_set(m_encodeCtx->priv_data, "profile", "baseline", 0);
    av_opt_set(m_encodeCtx->priv_data, "x264opts", "no-mbtree:sliced-threads:sync-lookahead=0", 0);

    m_encodeCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    m_encodeCtx->width = 1280;
    m_encodeCtx->height = 720;
    m_encodeCtx->time_base = (AVRational ) { 1, 24 };
    m_encodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_encodeCtx->bit_rate = 1000 * 1000;
    //m_encodeCtx->rc_max_rate = 300 * 1024;
    //m_encodeCtx->rc_buffer_size = 300 * 1024;
    m_encodeCtx->gop_size = 45;
    m_encodeCtx->max_b_frames = 2;
    m_encodeCtx->keyint_min = 45;
    m_encodeCtx->b_frame_strategy = 1;
    m_encodeCtx->coder_type = 1;
    m_encodeCtx->qmin = 15;
    m_encodeCtx->qmax = 25;
    m_encodeCtx->scenechange_threshold = 40;
    m_encodeCtx->flags |= CODEC_FLAG_LOOP_FILTER;
    m_encodeCtx->me_method = ME_HEX;
    m_encodeCtx->me_subpel_quality = 7;
    m_encodeCtx->me_cmp = 1;
    m_encodeCtx->me_range = 16;
    m_encodeCtx->i_quant_factor = 0.71;
    m_encodeCtx->qcompress = 0.6;
    m_encodeCtx->max_qdiff = 4;
    m_encodeCtx->thread_count = 2;
    //m_encodeCtx->profile = FF_PROFILE_H264_HIGH;

    m_fmtCtx->oformat->flags |= AVFMT_TS_NONSTRICT;

    if (m_fmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
    {
        m_encodeCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    DBG_CHECK((avcodec_open2(m_encodeCtx, codec, 0) < 0), return (VC_FAILURE), "Error: Unable to open codec %s", codec->long_name);

    av_dump_format(m_fmtCtx, 0, NULL, 1);
    m_output->OpenOutput(m_fmtCtx);
    avformat_write_header(m_fmtCtx, NULL);

    return (VC_SUCCESS);
}

VC_STATUS VideoEncoder::Uninitialize()
{
    DBG_MSG("Enter");

    avformat_close_input(&m_fmtCtx);
    avcodec_close(m_encodeCtx);
    m_output->CloseOutput();

    delete m_output;
    delete m_input;
    return (VC_SUCCESS);
}

VC_STATUS VideoEncoder::SendCommand(VC_CMD cmd)
{
    DBG_MSG("Enter");
    switch (cmd)
    {
    case VC_CMD_START:
        Start();
        break;
    case VC_CMD_STOP:
        AutoMutex automutex(&m_mutex);
        Stop();
        m_cv.Notify();
        Join();
        break;
    }
    return (VC_SUCCESS);
}

VC_STATUS VideoEncoder::SetParameters(const InputParams* params)
{
    return (VC_SUCCESS);
}

VC_STATUS VideoEncoder::GetParameters(OutputParams* params)
{
    return (VC_SUCCESS);
}

InputPort* VideoEncoder::Input(int portno)
{
    return (m_input);
}

OutputPort* VideoEncoder::Output(int portno)
{
    return (m_output);
}

VC_STATUS VideoEncoder::Notify(VC_EVENT* evt)
{
    AutoMutex automutex(&m_mutex);
    m_cv.Notify();
    return (VC_SUCCESS);
}

void VideoEncoder::Task()
{
    DBG_TRACE("Enter");

    int got_output;
    int err;

    int frames = 0;

    while (m_state)
    {
        if (m_input->IsBufferAvailable())
        {
            AVPacket pkt;
            av_init_packet(&pkt);
            pkt.data = NULL;
            pkt.size = 0;
            av_init_packet(&pkt);
            Buffer* inbuf = m_input->GetFilledBuffer();
            AVFrame* frame = static_cast<AVFrame*>((void*)inbuf->GetData());
            frame->pts = frames;

            err = avcodec_encode_video2(m_encodeCtx, &pkt, frame, &got_output);
            DBG_CHECK(err < 0, return, "Error(%d): Encoding frame", err);

            if (got_output)
            {
                pkt.stream_index = m_vidstream->index;
                if (pkt.pts != AV_NOPTS_VALUE)
                {
                    pkt.pts = av_rescale_q(pkt.pts, m_encodeCtx->time_base, m_vidstream->time_base);
                }
                if (pkt.dts != AV_NOPTS_VALUE)
                {
                    pkt.dts = av_rescale_q(pkt.dts, m_encodeCtx->time_base, m_vidstream->time_base);
                }

                if (m_encodeCtx->coded_frame->key_frame)
                {
                    pkt.flags |= AV_PKT_FLAG_KEY;
                }

                if (m_encodeCtx->codec->id == AV_CODEC_ID_H264)
                {
                    AVPacket new_pkt = pkt;
                    int result = av_bitstream_filter_filter(m_h264filter_ctx, m_fmtCtx->streams[pkt.stream_index]->codec, NULL,
                        &new_pkt.data, &new_pkt.size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);

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
                }

                av_interleaved_write_frame(m_fmtCtx, &pkt);
                frames++;
            }

            m_input->RecycleBuffer(inbuf);
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

    av_write_trailer(m_fmtCtx);
}

OutputCustomIO::OutputCustomIO(std::string name, ADevice* device) :
    OutputPort(name, device),
    m_ctx(NULL)
{
}

OutputCustomIO::~OutputCustomIO()
{
}

VC_STATUS OutputCustomIO::OpenOutput(AVFormatContext* ctx)
{
    const int buffer_size = 32 * 1024;
    ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
    ctx->pb = avio_alloc_context((unsigned char*) av_malloc(buffer_size), buffer_size, 0, this, NULL, write_cb, NULL);
    m_ctx = ctx;

    return (VC_SUCCESS);
}

VC_STATUS OutputCustomIO::CloseOutput()
{
    if (m_ctx && m_ctx->oformat)
    {
        av_free(m_ctx->pb->buffer);
        av_free(m_ctx->pb);
    }

    return (VC_SUCCESS);
}

int OutputCustomIO::Write(uint8_t *buf, int size)
{
    Buffer* outbuf = GetBuffer();
    outbuf->WriteData(buf, size);
    PushBuffer(outbuf);

    return (size);
}
int OutputCustomIO::write_cb(void *opaque, uint8_t *buf, int size)
{
    OutputCustomIO* self = static_cast<OutputCustomIO*>(opaque);
    return (self->Write(buf, size));
}
