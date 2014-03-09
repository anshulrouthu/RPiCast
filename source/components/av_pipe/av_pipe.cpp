/*
 * ap_piipe.cpp
 *
 *  Created on: Mar 9, 2014
 *      Author: anshul
 */

#include "av_pipe.h"
#include "capturedevice.h"
#include "video_encoder.h"
#include "file_io.h"

AVPipe::AVPipe(std::string name) :
    BasePipe(name)
{
}

ADevice* AVPipe::GetDevice(VC_DEVICETYPE devtype, std::string name, const char* filename)
{
    DBG_TRACE("Enter");
    switch (devtype)
    {
    case VC_CAPTURE_DEVICE:
        return (new CaptureDevice(name));
        break;
    case VC_VIDEO_ENCODER:
        return (new VideoEncoder(name));
        break;
    case VC_AUDIO_PROCESSOR:
        break;
    case VC_FLAC_DEVICE:
        break;
    case VC_CURL_DEVICE:
        break;
    case VC_TEXT_PROCESSOR:
        break;
    case VC_COMMAND_PROCESSOR:
        break;
    case VC_FILESINK_DEVICE:
        return (new FileSink(name, filename));
        break;
    case VC_FILESRC_DEVICE:
        return (new FileSrc(name, filename));
        break;
    default:
        break;
    }

    return (NULL);
}
