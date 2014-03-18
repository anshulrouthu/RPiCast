/*
 * test_demux.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: anshul
 */
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
    char* file = NULL;
    while ((c = getopt(argc, argv, "?l:d:s:f:a:t:")) != -1)
    {
        switch (c)
        {
        case 'd':
            DebugSetLevel(strtol(optarg, NULL, 10));
            break;
        case 'f':
            file = optarg;
            break;
        default:
            break;
        }
    }


    BasePipe* pipe = new AVPipe("Pipe 0");

    ADevice* fsrc = pipe->GetDevice(VC_FILESRC_DEVICE,"FileSrcDevice",file);
    ADevice* demux = pipe->GetDevice(VC_DEMUX_DEVICE,"DemuxDevice");
    ADevice* fsink = pipe->GetDevice(VC_FILESINK_DEVICE,"FileSinkDevice","Demux.out");

    fsrc->Initialize();
    demux->Initialize();
    fsink->Initialize();

    pipe->ConnectDevices(fsrc,demux);
    pipe->ConnectDevices(demux,fsink);

    fsrc->SendCommand(VC_CMD_START);
    demux->SendCommand(VC_CMD_START);
    fsink->SendCommand(VC_CMD_START);

    while(getch() != 'q');

    fsrc->SendCommand(VC_CMD_STOP);
    demux->SendCommand(VC_CMD_STOP);
    fsink->SendCommand(VC_CMD_STOP);

    pipe->DisconnectDevices(fsrc,demux);
    pipe->DisconnectDevices(demux,fsink);

    fsrc->Uninitialize();
    demux->Uninitialize();
    fsink->Uninitialize();

    delete fsrc;
    delete demux;
    delete fsink;
    delete pipe;

    return (0);
}





