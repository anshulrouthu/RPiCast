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
    {
        AutoMutex automutex(&m_mutex);
        m_cv.Notify();
    }

    Join();

}

VC_STATUS VideoEncoder::Initialize()
{
    AVCodec* codec;

    m_input = new InputPort("VidEncoder_Input", this);
    m_output = new OutputPort("VidEncoder_Output", this);

    /* find the mpeg1 video encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_MPEG2VIDEO);
    DBG_CHECK(!codec, return (VC_FAILURE), "Error: Encoder codec not found");

    m_encodeCtx = avcodec_alloc_context3(codec);
    DBG_CHECK(!m_encodeCtx, return (VC_FAILURE), "Error: Unable to allocate encoder codec context");

    /* put sample parameters */
    m_encodeCtx->bit_rate = 400000;
    m_encodeCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    m_encodeCtx->width = 1920;
    m_encodeCtx->height = 1080;
    m_encodeCtx->time_base = (AVRational ) { 1, 60 };
    m_encodeCtx->gop_size = 45;
    m_encodeCtx->max_b_frames = 1;
    m_encodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    m_encodeCtx->qmin = 3;
    m_encodeCtx->qmax = 6;

    DBG_CHECK((avcodec_open2(m_encodeCtx, codec, 0) < 0), return (VC_FAILURE), "Error: Unable to open codec %s",
        codec->long_name);

    return (VC_SUCCESS);
}

VC_STATUS VideoEncoder::Uninitialize()
{

    avcodec_close(m_encodeCtx);

    delete m_input;
    delete m_output;

    return (VC_SUCCESS);
}

VC_STATUS VideoEncoder::SendCommand(VC_CMD cmd)
{
    switch (cmd)
    {
    case VC_CMD_START:
        Start();
        break;
    case VC_CMD_STOP:
        Stop();
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
            Buffer* outbuf = m_output->GetBuffer();

            AVFrame* frame = static_cast<AVFrame*>(inbuf->GetData());

            err = avcodec_encode_video2(m_encodeCtx, &pkt, frame, &got_output);
            DBG_CHECK(err < 0, return, "Error(%d): Encoding frame", err);

            if (got_output)
            {
                outbuf->WriteData(pkt.data, pkt.size);
                av_free_packet(&pkt);
            }

            /* get the delayed frames */
            for (int i = 0, got_output = 1; got_output; i++)
            {
                err = avcodec_encode_video2(m_encodeCtx, &pkt, 0, &got_output);
                DBG_CHECK(err < 0, return, "Error(%d): Encoding delayed frame", err);

                if (got_output)
                {
                    outbuf->WriteData(pkt.data, pkt.size);
                    av_free_packet(&pkt);
                }
            }

            m_output->PushBuffer(outbuf);
            m_input->RecycleBuffer(inbuf);
        }
        else
        {
            AutoMutex automutex(&m_mutex);
            while(!m_input->IsBufferAvailable() && m_state)
            {
                m_cv.Wait();
            }
        }
    }
}
