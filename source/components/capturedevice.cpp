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

#include "capturedevice.h"

#define MAX_DEFAULT_HEIGHT 1080
#define MAX_DEFAULT_WIDTH 1920

/**
 * Capturedevice constructor. Initializes the threashold level of input audio level
 * @param thr threshold input level
 */
CaptureDevice::CaptureDevice(std::string name) :
    ADevice(name),
    m_running(false),
    m_timer(NULL),
    m_input(NULL),
    m_output(NULL)
{
}

/**
 * Capturedevice Destructor
 */
CaptureDevice::~CaptureDevice()
{
    /* this mutex is required by the main thread waiting on a condition
     take the mutex wake up main thread and release the mutex */
    {
        AutoMutex automutex(&m_mutex);
        m_cv.Notify();
    }

    Join();

    delete m_timer;
    delete m_input;
    delete m_output;

}

VC_STATUS CaptureDevice::Uninitialize()
{
    avcodec_close(m_decodeCtx);
    avformat_close_input(&m_fmtCtx);

    return (VC_SUCCESS);
}

/**
 * Initialize the device and get all the resources
 */
VC_STATUS CaptureDevice::Initialize()
{
    DBG_TRACE("Enter");

    AVCodec* codec;
    AVDictionary* options = NULL;
    AVInputFormat * avfmt;
    int err;
    int vid_streamid = -1;

    m_timer = new Timer();
    m_input = new InputPort("VidCap Input 0", this);
    m_output = new OutputPort("VidCap Ouput 0", this);

    // Register all formats and codecs
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();

    av_dict_set(&options, "framerate", "60", 0);
    av_dict_set(&options, "video_size", "1920x1080", 0);
    av_dict_set(&options, "qscale", "1", 0);

    avfmt = av_find_input_format("x11grab");

    err = avformat_open_input(&m_fmtCtx, ":0.0", avfmt, &options);
    DBG_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Opening format input", err);

    err = avformat_find_stream_info(m_fmtCtx, NULL);
    DBG_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Finding stream info", err);

    // Dump information about file onto standard error
    av_dump_format(m_fmtCtx, 0, NULL, 0);

    for (uint i = 0; i < m_fmtCtx->nb_streams; i++)
    {
        if (m_fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vid_streamid = i;
            break;
        }
    }
    DBG_CHECK(vid_streamid == -1, return (VC_FAILURE), "Error: Unable to find video stream");

    m_decodeCtx = m_fmtCtx->streams[vid_streamid]->codec;

    codec = avcodec_find_decoder(m_decodeCtx->codec_id);
    DBG_CHECK(codec == NULL, return (VC_FAILURE), "Error: Finding video decoder");

    DBG_CHECK((avcodec_open2(m_decodeCtx, codec, 0) < 0), return (VC_FAILURE), "Error: Unable to open codec %s",
        codec->long_name);

    return (VC_SUCCESS);
}

/**
 * Return the input port of the device
 * @return m_input
 */
InputPort* CaptureDevice::Input(int portno)
{
    return (m_input);
}

/**
 * Return the output port of the device
 * @return m_output
 */
OutputPort* CaptureDevice::Output(int portno)
{
    return (m_output);
}

/**
 * Notifies the device of any event
 */
VC_STATUS CaptureDevice::Notify(VC_EVENT* evt)
{
    //TODO: update the api to notify different type of events
    AutoMutex automutex(&m_mutex);
    m_cv.Notify();

    return (VC_SUCCESS);
}

/**
 * Send a command to the device
 */
VC_STATUS CaptureDevice::SendCommand(VC_CMD cmd)
{
    switch (cmd)
    {
    case VC_CMD_START:
        Start();
        //usleep(10000);
        m_running = true;
        break;
    case VC_CMD_STOP:
        m_running = false;
        Stop();
        Join();
        break;
    }
    return (VC_SUCCESS);
}

/**
 * Set the required parameters for device
 * @paran params
 */
VC_STATUS CaptureDevice::SetParameters(const InputParams* params)
{
    return (VC_SUCCESS);
}

/**
 * Get the required parameters from device
 * @paran params
 */
VC_STATUS CaptureDevice::GetParameters(OutputParams* params)
{
    return (VC_SUCCESS);
}

/**
 * Thread that captures data.
 * This threads looks for any valid data with amplitude more than
 * threshold value, and fulls the buffer with data and sends to audioprocessor module
 */
void CaptureDevice::Task()
{
    DBG_MSG("Enter");
    AVPacket packet;
    AVFrame* bgr_frame;
    AVFrame* yuv_frame;
    struct SwsContext * swsCtx;
    int frameFinished;
    int pts;

    bgr_frame = avcodec_alloc_frame();
    //yuv_frame = avcodec_alloc_frame();

    // Determine required buffer size and allocate buffer
    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, MAX_DEFAULT_WIDTH, MAX_DEFAULT_HEIGHT);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture

    swsCtx = sws_getContext(MAX_DEFAULT_WIDTH, MAX_DEFAULT_HEIGHT, m_decodeCtx->pix_fmt, MAX_DEFAULT_WIDTH,
        MAX_DEFAULT_HEIGHT, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

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

                    avpicture_fill((AVPicture *) yuv_frame, buffer, AV_PIX_FMT_YUV420P, MAX_DEFAULT_WIDTH, MAX_DEFAULT_HEIGHT);
                    /* convert frame in to yuv */
                    sws_scale(swsCtx, (const uint8_t * const *) bgr_frame->data, bgr_frame->linesize, 0, MAX_DEFAULT_HEIGHT, yuv_frame->data, yuv_frame->linesize);
                    yuv_frame->pts = pts;
                    m_output->PushBuffer(buf);
                }
                av_free_packet(&packet);
            }
        }

        AutoMutex automutex(&m_mutex);
        m_cv.Wait(100);
    }

    av_free(bgr_frame);
}
