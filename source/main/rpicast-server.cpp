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

int main(int argc, char* argv[])
{
    int c;
    char* filename = NULL;

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
    ADevice* src;

    if (!filename)
    {
        src = pipe->GetDevice(VC_SOCKET_RECEIVER, "SocketReceiver", filename);
    }
    else
    {
        src = pipe->GetDevice(VC_FILESRC_DEVICE, "FileSrc", filename);
    }

    ADevice* demux = pipe->GetDevice(VC_DEMUX_DEVICE, "DemuxDevice");
    ADevice* sink = pipe->GetDevice(VC_VIDEO_TUNNEL, "VideoTunnel");

    src->Initialize();
    demux->Initialize();
    DBG_CHECK_STATIC(sink->Initialize() != VC_SUCCESS, return (0), "Error Exiting");

    pipe->ConnectDevices(src, demux);
    pipe->ConnectDevices(demux, sink);

    src->SendCommand(VC_CMD_START);
    demux->SendCommand(VC_CMD_START);
    sink->SendCommand(VC_CMD_START);

    while (getch() != 'q')
        ;

    src->SendCommand(VC_CMD_STOP);
    demux->SendCommand(VC_CMD_STOP);
    sink->SendCommand(VC_CMD_STOP);

    pipe->DisconnectDevices(src, demux);
    pipe->DisconnectDevices(demux, sink);

    src->Uninitialize();
    demux->Uninitialize();
    sink->Uninitialize();

    delete src;
    delete demux;
    delete sink;
    delete pipe;

    return (0);
}

