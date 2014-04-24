/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file av_pipe.h is part of RPiCast project

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

#ifndef AVPIPE_H_
#define AVPIPE_H_

#include "basepipe.h"
#include "video_capture.h"
#include "video_encoder.h"
#include "file_io.h"
#include "socket_io.h"
#include "demux.h"
#ifdef RPI_BUILD
#include "video_tunnel.h"
#endif

class AVPipe: public BasePipe
{
public:
    AVPipe(std::string name);
    virtual ~AVPipe()
    {
    }
    virtual ADevice* GetDevice(VC_DEVICETYPE devtype, std::string name, const char* filename);
};

#endif /* AVPIPE_H_ */
