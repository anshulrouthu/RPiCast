/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * voiceCommand.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */
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

    while (getch() != 'q')
        ;

    pipe->SendCommand(VC_CMD_STOP);
    pipe->Reset();
    pipe->Uninitialize();

    delete pipe;

    return (0);
}

