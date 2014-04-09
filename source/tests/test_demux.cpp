/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file test_demux.cpp is part of RPiCast project

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

    ADevice* fsrc = pipe->GetDevice(VC_FILESRC_DEVICE, "FileSrcDevice", file);
    ADevice* demux = pipe->GetDevice(VC_DEMUX_DEVICE, "DemuxDevice");
    ADevice* fsink = pipe->GetDevice(VC_FILESINK_DEVICE, "FileSinkDevice", "Demux.out");

    fsrc->Initialize();
    demux->Initialize();
    fsink->Initialize();

    pipe->ConnectDevices(fsrc, demux);
    pipe->ConnectDevices(demux, fsink);

    fsrc->SendCommand(VC_CMD_START);
    demux->SendCommand(VC_CMD_START);
    fsink->SendCommand(VC_CMD_START);

    while (getch() != 'q');

    fsrc->SendCommand(VC_CMD_STOP);
    demux->SendCommand(VC_CMD_STOP);
    fsink->SendCommand(VC_CMD_STOP);

    pipe->DisconnectDevices(fsrc, demux);
    pipe->DisconnectDevices(demux, fsink);

    fsrc->Uninitialize();
    demux->Uninitialize();
    fsink->Uninitialize();

    delete fsrc;
    delete demux;
    delete fsink;
    delete pipe;

    return (0);
}

