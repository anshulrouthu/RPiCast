/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file av_pipe.cpp is part of RPiCast project

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
 *********************************************************************/

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
    case VC_DEMUX_DEVICE:
        return (new DemuxDevice(name, this));
        break;
#ifdef ARM_CROSS
        case VC_VIDEO_TUNNEL:
        return (new VideoTunnel(name));
#endif
    default:
        break;
    }

    return (NULL);
}
