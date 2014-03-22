/*
 * video_encoder.cpp
 *
 *  Created on: Mar 9, 2014
 *      Author: anshul
 */

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

    codec = avcodec_find_encoder(AV_CODEC_ID_MPEG2VIDEO);
    DBG_CHECK(!codec, return (VC_FAILURE), "Error: Encoder codec not found");

    m_fmtCtx = avformat_alloc_context();
    m_fmtCtx->oformat = av_guess_format(NULL, "out.m2ts", NULL);
    m_vidstream = avformat_new_stream(m_fmtCtx, codec);

    m_encodeCtx = m_vidstream->codec;
    DBG_CHECK(!m_encodeCtx, return (VC_FAILURE), "Error: Unable to allocate encoder codec context");

    m_encodeCtx->bit_rate = 400000;
    m_encodeCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    m_encodeCtx->width = 1280;
    m_encodeCtx->height = 720;
    m_encodeCtx->time_base = (AVRational ) { 1, 30 };
    m_encodeCtx->gop_size = 45;
    m_encodeCtx->max_b_frames = 1;
    m_encodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_encodeCtx->qmin = 8;
    m_encodeCtx->qmax = 12;
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
    AVPacket pkt;

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    while (m_state)
    {
        if (m_input->IsBufferAvailable())
        {
            Buffer* inbuf = m_input->GetFilledBuffer();
            AVFrame* frame = static_cast<AVFrame*>(inbuf->GetData());

            err = avcodec_encode_video2(m_encodeCtx, &pkt, frame, &got_output);
            DBG_CHECK(err < 0, return, "Error(%d): Encoding frame", err);

            for (int i = 0, got_output = 1; got_output; i++)
            {
                err = avcodec_encode_video2(m_encodeCtx, &pkt, 0, &got_output);
                DBG_CHECK(err < 0, return, "Error(%d): Encoding delayed frame", err);

                if (got_output)
                {
                    if (m_encodeCtx->coded_frame->key_frame)
                    {
                        pkt.flags |= AV_PKT_FLAG_KEY;
                    }

                    pkt.pts = av_rescale_q(m_encodeCtx->coded_frame->pts, m_encodeCtx->time_base, m_vidstream->time_base);
                    pkt.stream_index = m_vidstream->index;
                    av_interleaved_write_frame(m_fmtCtx, &pkt);
                    av_free_packet(&pkt);
                }
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
}

OutputCustomIO::OutputCustomIO(std::string name, ADevice* device):
    OutputPort(name,device),
    m_ctx(NULL)
{
}

OutputCustomIO::~OutputCustomIO()
{
}

VC_STATUS OutputCustomIO::OpenOutput(AVFormatContext* ctx)
{
    const int buffer_size = 1024 * 1024;
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
    return(self->Write(buf,size));
}
