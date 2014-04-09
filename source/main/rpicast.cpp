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

void printSSDPServers(SSDPServerList list)
{
    int i = 1;
    for (SSDPServerList::iterator it = list.begin(); it != list.end(); it++)
    {
        DBGPRINT(LEVEL_ALWAYS, ("%d. %s %s\n",i,(*it)->name.c_str(),(*it)->ip.c_str()));
        i++;
    }
}

int main(int argc, char* argv[])
{
    int c;
    const char* address = NULL;
    char* file = NULL;
    bool screen_cast = true;

    SSDPServerList list;
    SSDPClient ssdp_c("SSDPClient");
    ssdp_c.SearchDevices(list);

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

    if (list.size())
    {
        DBGPRINT(LEVEL_ALWAYS, ("\nFound Devices\n"));
        printSSDPServers(list);
    }

    bool loop = true;
    while (loop)
    {
        DBGPRINT(LEVEL_ALWAYS, ("\n\tMENU\n"));
        DBGPRINT(LEVEL_ALWAYS, ("1. Search for RpiCast\n"));
        DBGPRINT(LEVEL_ALWAYS, ("2. Cast Screen to RpiCast\n"));
        DBGPRINT(LEVEL_ALWAYS, ("3. Capture to a file\n"));
        char c = getch();
        switch (c)
        {
        case '1':
            ssdp_c.SearchDevices(list);
            DBGPRINT(LEVEL_ALWAYS, ("\nFound Devices\n"));
            printSSDPServers(list);
            break;
        case '2':
            if (list.size() > 1)
            {
                int d;
                DBGPRINT(LEVEL_ALWAYS, ("\nPlease select a device\n"));
                printSSDPServers(list);
                scanf("%d", &d);
                int i = 1;
                for (SSDPServerList::iterator it = list.begin(); it != list.end(); it++)
                {
                    if (i == d)
                    {
                        address = (*it)->ip.c_str();
                        DBGPRINT(LEVEL_ALWAYS, ("Casting to %s %s\n",(*it)->name.c_str(),(*it)->ip.c_str()));
                        break;
                    }
                    i++;
                }

            }
            else
            {
                address = list.front()->ip.c_str();
            }

            loop = false;
            break;
        case '3':
            address = NULL;
            loop = false;
            break;
        case 'q':
            exit(0);
        default:
            break;
        }

    }

    BasePipe* pipe = new AVPipe("Pipe 0");

    if (screen_cast)
    {
        pipe->AddDevice(VC_CAPTURE_DEVICE, "Capture 0");
        pipe->AddDevice(VC_VIDEO_ENCODER, "VidEncoder 0");
    }
    else
    {
        pipe->AddDevice(VC_FILESRC_DEVICE, "FileSrc", file);
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

    while (getch() != 'q')
        ;

    pipe->SendCommand(VC_CMD_STOP);
    pipe->Reset();
    pipe->Uninitialize();

    delete pipe;

    return (0);
}

