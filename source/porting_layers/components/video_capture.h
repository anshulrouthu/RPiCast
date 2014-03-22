/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/**
 * @file capturedevice.h
 *
 * Capturedevice is the main source for the audio data to the application.
 * This device captured the audio data when ever there is any
 *
 *   Created on: Jan 9, 2014
 *       Author: anshul routhu <anshul.m67@gmail.com>
 */

#ifndef VIDEO_CAPTURE_H_
#define VIDEO_CAPTURE_H_


extern "C"
{
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#include "utils.h"
#include "worker.h"
#include "buffer.h"
#include "basepipe.h"
#include "timer.h"
#include "mutex.h"

/**
 * Capture Device Class that captures audio data continuously.
 * This class is responsible for capture of raw audio pcm data
 * This is a wrapper function around OpenAL library
 */
class VideoCapture: public WorkerThread, public ADevice
{
public:
    VideoCapture(std::string name);
    virtual ~VideoCapture();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Uninitialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno);
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);
    virtual VC_STATUS SetParameters(const InputParams* params);
    virtual VC_STATUS GetParameters(OutputParams* params);

private:
    virtual void Task();

    AVFormatContext* m_fmtCtx;
    AVCodecContext* m_decodeCtx;

    InputPort* m_input;
    OutputPort* m_output;

};

#endif /* VIDEO_CAPTURE_H_ */
