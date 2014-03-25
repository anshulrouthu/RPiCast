/*
 * video_encoder.h
 *
 *  Created on: Mar 9, 2014
 *      Author: anshul
 */

#ifndef VIDEO_ENCODER_H_
#define VIDEO_ENCODER_H_

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

#include "basepipe.h"
#include "utils.h"
#include "worker.h"

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

class OutputCustomIO : public OutputPort
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
