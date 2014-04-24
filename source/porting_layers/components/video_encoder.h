/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file video_encoder.h is part of RPiCast project

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
 ***********************************************************/

#ifndef VIDEO_ENCODER_H_
#define VIDEO_ENCODER_H_

#include "basepipe.h"
#include "utils.h"
#include "worker.h"

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
#include <libavutil/opt.h>
}

class OutputCustomIO;

class VideoEncoder: public ADevice, public WorkerThread
{
public:
    VideoEncoder(std::string name);
    ~VideoEncoder();

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
    AVCodecContext* m_encodeCtx;
    InputPort* m_input;
    //OutputPort* m_output;
    OutputCustomIO* m_output;
    AVStream* m_vidstream;
    AVBitStreamFilterContext* m_h264filter_ctx;

};

class OutputCustomIO: public OutputPort
{
public:
    OutputCustomIO(std::string name, ADevice* device);
    virtual ~OutputCustomIO();

    VC_STATUS OpenOutput(AVFormatContext* ctx);
    VC_STATUS CloseOutput();

private:
    int Write(uint8_t *pBuffer, int pBufferSize);
    static int write_cb(void *opaque, uint8_t *pBuffer, int pBufferSize);
    AVFormatContext* m_ctx;
};

#endif /* VIDEO_ENCODER_H_ */
