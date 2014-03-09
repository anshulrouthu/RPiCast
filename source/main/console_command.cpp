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
    while ((c = getopt(argc, argv, "sf?l:d:t:")) != -1)
    {
        switch (c)
        {
        case 'd':
            DebugSetLevel(strtol(optarg, NULL, 10));
            break;
        default:
            break;
        }
    }


    BasePipe* pipe = new AVPipe("Pipe 0");

    ADevice* src = pipe->GetDevice(VC_CAPTURE_DEVICE, "Capture 0");
    ADevice* encoder = pipe->GetDevice(VC_VIDEO_ENCODER,"VidEncoder 0");
    ADevice* sink = pipe->GetDevice(VC_FILESINK_DEVICE, "FileSink 0", "ScreenCap.out");

    src->Initialize();
    sink->Initialize();
    encoder->Initialize();

    pipe->ConnectDevices(src,encoder);
    pipe->ConnectDevices(encoder,sink);

    src->SendCommand(VC_CMD_START);
    encoder->SendCommand(VC_CMD_START);
    sink->SendCommand(VC_CMD_START);

    while(getch() != 'q');

    src->SendCommand(VC_CMD_STOP);
    encoder->SendCommand(VC_CMD_STOP);
    sink->SendCommand(VC_CMD_STOP);

    pipe->DisconnectDevices(src,encoder);
    pipe->DisconnectDevices(encoder,sink);

    src->Uninitialize();
    encoder->Uninitialize();
    sink->Uninitialize();

    delete src;
    delete encoder;
    delete sink;
    delete pipe;


    return (0);
}

