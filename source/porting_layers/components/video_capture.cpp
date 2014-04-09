/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/**
 * @file capturedevice.cpp
 *
 * Capturedevice is the main source for the audio data to the application.
 * This device captured the audio data when ever there is any
 *
 *   Created on: Jan 9, 2014
 *       Author: anshul routhu <anshul.m67@gmail.com>
 */

#include "video_capture.h"

#define MAX_DEFAULT_HEIGHT 1080
#define MAX_DEFAULT_WIDTH 1920
#define SCALED_HEIGHT 720
#define SCALED_WIDTH 1280

/**
 * Capturedevice constructor. Initializes the threashold level of input audio level
 * @param thr threshold input level
 */
VideoCapture::VideoCapture(std::string name) :
    ADevice(name),
    m_input(NULL),
    m_output(NULL)
{
}

/**
 * Capturedevice Destructor
 */
VideoCapture::~VideoCapture()
{
    DBG_MSG("Enter");

}

VC_STATUS VideoCapture::Uninitialize()
{
    DBG_MSG("Enter");
    avcodec_close(m_decodeCtx);
    avformat_close_input(&m_fmtCtx);

    delete m_input;
    delete m_output;

    return (VC_SUCCESS);
}

/**
 * Initialize the device and get all the resources
 */
VC_STATUS VideoCapture::Initialize()
{
    DBG_MSG("Enter");

    AVCodec* codec;
    AVDictionary* options = NULL;
    AVInputFormat * avfmt;
    int err;

    m_input = new InputPort("VidCap Input 0", this);
    m_output = new OutputPort("VidCap Ouput 0", this);

    av_register_all();
    avcodec_register_all();
    avdevice_register_all();

    av_dict_set(&options, "video_size", "1920x1080", 0);
    av_dict_set(&options, "pix_fmt", "yuv420p", 0);

    avfmt = av_find_input_format("x11grab");

    err = avformat_open_input(&m_fmtCtx, ":0.0", avfmt, &options);
    DBG_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Opening format input", err);

    err = avformat_find_stream_info(m_fmtCtx, NULL);
    DBG_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Finding stream info", err);

    av_dump_format(m_fmtCtx, 0, NULL, 0);

    for (uint i = 0; i < m_fmtCtx->nb_streams; i++)
    {
        if (m_fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_decodeCtx = m_fmtCtx->streams[i]->codec;
            break;
        }
    }
    DBG_CHECK(!m_decodeCtx, return (VC_FAILURE), "Error: Unable to find video stream");

    codec = avcodec_find_decoder(m_decodeCtx->codec_id);
    DBG_CHECK(codec == NULL, return (VC_FAILURE), "Error: Finding video decoder");

    DBG_CHECK((avcodec_open2(m_decodeCtx, codec, 0) < 0), return (VC_FAILURE), "Error: Unable to open codec %s", codec->long_name);

    return (VC_SUCCESS);
}

/**
 * Return the input port of the device
 * @return m_input
 */
InputPort* VideoCapture::Input(int portno)
{
    return (m_input);
}

/**
 * Return the output port of the device
 * @return m_output
 */
OutputPort* VideoCapture::Output(int portno)
{
    return (m_output);
}

/**
 * Notifies the device of any event
 */
VC_STATUS VideoCapture::Notify(VC_EVENT* evt)
{
    //TODO: update the api to notify different type of events
    AutoMutex automutex(&m_mutex);
    m_cv.Notify();

    return (VC_SUCCESS);
}

/**
 * Send a command to the device
 */
VC_STATUS VideoCapture::SendCommand(VC_CMD cmd)
{
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

/**
 * Set the required parameters for device
 * @paran params
 */
VC_STATUS VideoCapture::SetParameters(const InputParams* params)
{
    return (VC_SUCCESS);
}

/**
 * Get the required parameters from device
 * @paran params
 */
VC_STATUS VideoCapture::GetParameters(OutputParams* params)
{
    return (VC_SUCCESS);
}

/**
 * Thread that captures data.
 * This threads looks for any valid data with amplitude more than
 * threshold value, and fulls the buffer with data and sends to audioprocessor module
 */
void VideoCapture::Task()
{
    DBG_MSG("Enter");
    AVPacket packet;
    AVFrame* bgr_frame;
    AVFrame* yuv_frame;
    struct SwsContext * swsCtx;
    int frameFinished;
    int64_t pts;

    bgr_frame = avcodec_alloc_frame();

    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, SCALED_WIDTH, SCALED_HEIGHT);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    swsCtx = sws_getContext(MAX_DEFAULT_WIDTH, MAX_DEFAULT_HEIGHT, m_decodeCtx->pix_fmt, SCALED_WIDTH, SCALED_HEIGHT, AV_PIX_FMT_YUV420P,
        SWS_FAST_BILINEAR, NULL, NULL, NULL);

    while (m_state)
    {
        if (av_read_frame(m_fmtCtx, &packet) >= 0)
        {
            if (m_fmtCtx->streams[packet.stream_index]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                avcodec_decode_video2(m_decodeCtx, bgr_frame, &frameFinished, &packet);

                if (frameFinished)
                {
                    pts = packet.dts != AV_NOPTS_VALUE ? packet.pts : 0;
                    pts *= av_q2d(m_decodeCtx->time_base);

                    Buffer* buf = m_output->GetBuffer();
                    yuv_frame = static_cast<AVFrame*>(buf->GetData());
                    avpicture_fill((AVPicture *) yuv_frame, buffer, AV_PIX_FMT_YUV420P, SCALED_WIDTH, SCALED_HEIGHT);

                    sws_scale(swsCtx, (const uint8_t * const *) bgr_frame->data, bgr_frame->linesize, 0, MAX_DEFAULT_HEIGHT,
                        yuv_frame->data, yuv_frame->linesize);
                    yuv_frame->pts = pts;
                    m_output->PushBuffer(buf);
                }
                av_free_packet(&packet);
            }
        }
    }

    av_free(bgr_frame);
}
