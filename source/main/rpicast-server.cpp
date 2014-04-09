/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file rpicast-server.cpp is part of RPiCast project

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

#include "utils.h"
#include <stdio.h>
#include "av_pipe.h"
#include "ssdpdiscovery.h"

int main(int argc, char* argv[])
{
    int c;
    char* filename = NULL;

    SSDPServer ssdp_s("SSDPServer");

    while ((c = getopt(argc, argv, "s?l:d:f:t:")) != -1)
    {
        switch (c)
        {
        case 'd':
            DebugSetLevel(strtol(optarg, NULL, 10));
            break;
        case 'f':
            filename = optarg;
            break;
        default:
            break;
        }
    }

    BasePipe* pipe = new AVPipe("Pipe 0");

    if (!filename)
    {
        pipe->AddDevice(VC_SOCKET_RECEIVER, "SocketReceiver", filename);
    }
    else
    {
        pipe->AddDevice(VC_FILESRC_DEVICE, "FileSrc", filename);
    }

    pipe->AddDevice(VC_DEMUX_DEVICE, "DemuxDevice");
    pipe->AddDevice(VC_VIDEO_TUNNEL, "VideoTunnel");

    pipe->Initialize();
    pipe->Prepare();
    pipe->SendCommand(VC_CMD_START);

    while (getch() != 'q');

    pipe->SendCommand(VC_CMD_STOP);
    pipe->Reset();
    pipe->Uninitialize();

    delete pipe;

    return (0);
}

