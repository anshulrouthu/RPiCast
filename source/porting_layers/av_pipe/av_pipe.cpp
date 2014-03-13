/*
 * ap_piipe.cpp
 *
 *  Created on: Mar 9, 2014
 *      Author: anshul
 */

#include "av_pipe.h"

AVPipe::AVPipe(std::string name) :
    BasePipe(name)
{
}

ADevice* AVPipe::GetDevice(VC_DEVICETYPE devtype, std::string name, const char* args)
{
    DBG_TRACE("Enter");
    switch (devtype)
    {
    case VC_CAPTURE_DEVICE:
        return (new VideoCapture(name));
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
        return (new FileSink(name, args));
        break;
    case VC_FILESRC_DEVICE:
        return (new FileSrc(name, args));
        break;
    case VC_SOCKET_RECEIVER:
        return (new SocketInDevice(name, this));
    case VC_SOCKET_TRANSMITTER:
        return (new SocketOutDevice(name, this, args));
    default:
        break;
    }

    return (NULL);
}
