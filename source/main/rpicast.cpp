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
    char* address = NULL;
    char* file = NULL;
    bool screen_cast = true;
    while ((c = getopt(argc, argv, "?l:d:s:f:a:t:")) != -1)
    {
        switch (c)
        {
        case 'd':
            DebugSetLevel(strtol(optarg, NULL, 10));
            break;
        case 'a':
            address = optarg;
            break;
        case 'f':
            file = optarg;
            screen_cast = false;
            break;
        default:
            break;
        }
    }


    BasePipe* pipe = new AVPipe("Pipe 0");

    ADevice* src;
    ADevice* capture;
    ADevice* sink;

    if (address)
    {
        sink = pipe->GetDevice(VC_SOCKET_TRANSMITTER, "SocketTx", address);
    }
    else
    {
        sink = pipe->GetDevice(VC_FILESINK_DEVICE, "FileSink", "Output.vid");
    }

    if(screen_cast)
    {
        capture = pipe->GetDevice(VC_CAPTURE_DEVICE, "Capture 0");
        src = pipe->GetDevice(VC_VIDEO_ENCODER,"VidEncoder 0");
        capture->Initialize();
        src->Initialize();
        pipe->ConnectDevices(capture,src);
    }
    else
    {
        src = pipe->GetDevice(VC_FILESRC_DEVICE, "FileSrc",file);
        src->Initialize();
    }

    sink->Initialize();
    pipe->ConnectDevices(src,sink);

    DBG_CHECK_STATIC(sink->SendCommand(VC_CMD_START) == VC_FAILURE,exit(0),"Error: Unable to connect to server");
    src->SendCommand(VC_CMD_START);
    if(screen_cast)
    {
        capture->SendCommand(VC_CMD_START);
    }

    while(getch() != 'q');


    if(screen_cast)
    {
        capture->SendCommand(VC_CMD_STOP);
    }

    src->SendCommand(VC_CMD_STOP);
    sink->SendCommand(VC_CMD_STOP);

    if(screen_cast)
    {
        pipe->DisconnectDevices(capture,src);
        capture->Uninitialize();
        delete capture;
    }

    pipe->DisconnectDevices(src,sink);

    src->Uninitialize();
    sink->Uninitialize();

    delete src;
    delete sink;
    delete pipe;


    return (0);
}

