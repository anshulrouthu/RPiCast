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

    if(screen_cast)
    {
        pipe->AddDevice(VC_CAPTURE_DEVICE, "Capture 0");
        pipe->AddDevice(VC_VIDEO_ENCODER,"VidEncoder 0");
    }
    else
    {
        pipe->AddDevice(VC_FILESRC_DEVICE, "FileSrc",file);
    }

    if (address)
    {
        pipe->AddDevice(VC_SOCKET_TRANSMITTER, "SocketTx", address);
    }
    else
    {
        pipe->AddDevice(VC_FILESINK_DEVICE, "FileSink", "Output.vid");
    }

    pipe->Initialize();
    pipe->Prepare();
    pipe->SendCommand(VC_CMD_START);

    while(getch() != 'q');

    pipe->SendCommand(VC_CMD_STOP);
    pipe->Reset();
    pipe->Uninitialize();

    delete pipe;


    return (0);
}

