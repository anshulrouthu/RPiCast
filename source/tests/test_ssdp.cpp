/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file test_ssdp.cpp is part of RPiCast project

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
    bool server = false;
    SSDPServer* ssdp_s = NULL;
    SSDPClient* ssdp_c = NULL;

    while ((c = getopt(argc, argv, "?sl:d:f:a:t:")) != -1)
    {
        switch (c)
        {
        case 'd':
            DebugSetLevel(strtol(optarg, NULL, 10));
            break;
        case 's':
            server = true;
            break;
        default:
            break;
        }
    }

    if (server)
    {
        ssdp_s = new SSDPServer("SSDPServer");
    }
    else
    {
        SSDPServerList list;
        ssdp_c = new SSDPClient("SSDPClient");
        ssdp_c->SearchDevices(list);

        if (list.size())
        {
            printf("Found Servers: \n");
            for (SSDPServerList::iterator it = list.begin(); it != list.end(); it++)
            {
                DBGPRINT(LEVEL_ALWAYS, ("%s %s\n",(*it)->name.c_str(),(*it)->ip.c_str()));
            }
        }
    }

    while (getch() != 'q');

    if (ssdp_s)
        delete ssdp_s;
    if (ssdp_c)
        delete ssdp_c;

    return (0);
}
