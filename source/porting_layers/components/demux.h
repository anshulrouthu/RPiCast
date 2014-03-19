/*
 * demux.h
 *
 *  Created on: Mar 17, 2014
 *      Author: anshul
 */

#ifndef DEMUX_H_
#define DEMUX_H_

#include "utils.h"
#include "basepipe.h"
#include "worker.h"
#include "buffer.h"
#include <map>
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

class CustomIOPort;

class DemuxDevice: public ADevice, public WorkerThread
{
public:
    DemuxDevice(std::string name, BasePipe* pipe);
    virtual ~DemuxDevice();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Uninitialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno = 0);
    virtual OutputPort* Output(int portno = 0);
    virtual VC_STATUS SendCommand(VC_CMD cmd);

private:
    virtual void Task();
    CustomIOPort* m_input;
    std::map<int, OutputPort*> m_output;
    AVFormatContext* m_ctx;
    AVBitStreamFilterContext* m_h264filter_ctx;
    bool m_preroll;
};

class CustomIOPort: public InputPort
{
    friend class DemuxDevice;
public:
    CustomIOPort(std::string name, ADevice* device);
    virtual ~CustomIOPort();
    VC_STATUS OpenInput(AVFormatContext* const ctx);
    VC_STATUS CloseInput();

private:
    int read(uint8_t *buf, int buf_size);
    static int read_cb(void *opaque, uint8_t *buf, int buf_size);
    static int64_t seek_cb(void *opaque, int64_t offset, int whence);
    AVFormatContext* m_ctx;
    Buffer* m_buffer;
    size_t m_rd_offset;
    Mutex m_wait_mutex;
    ConditionVariable m_wait_cv;
    bool m_initialized;
};

#endif /* DEMUX_H_ */
