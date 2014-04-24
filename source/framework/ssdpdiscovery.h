/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file ssdpdiscovery.h is part of RPiCast project

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
 ***********************************************************/

#ifndef SSDPDISCOVERY_H_
#define SSDPDISCOVERY_H_

#include "utils.h"
#include "buffer.h"
#include "basepipe.h"
#include "worker.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

#define SSDP_PORT 1900
#define SSDP_GROUP "239.255.255.250"
#define SSDP_STRING_SIZE 1024
#define SSDP_DISCOVERY_REQUEST  "M-SEARCH * HTTP/1.1\r\n"         \
                                 "HOST: 239.255.255.250:1900\r\n"  \
                                 "MAN: \"ssdp:discover\"\r\n"      \
                                 "MX: 10\r\n"                       \
                                 "ST: urn:dial-multiscreen-org:service:rpicast:1\r\n\r\n"

/*#define SSDP_DISCOVERY_RESPONSE "HTTP/1.1 200 OK\r\n"
//                                 "LOCATION: http://%s:%d/dd.xml\r\n"
//                                 "CACHE-CONTROL: max-age=1800\r\n"
//                                 "EXT:\r\n"
//                                 "BOOTID.UPNP.ORG: 1\r\n"
//                                 "SERVER: Linux/2.6 UPnP/1.0 quick_ssdp/1.0\r\n"
//                                 "ST: urn:dial-multiscreen-org:service:rpicast:1\r\n"
//                                 "USN: uuid:%s::urn:dial-multiscreen-org:service:rpicast:1\r\n\r\n"; */

#define SSDP_DEVICE_NAME "RPiCast"

typedef struct ssdp_server
{
    std::string name;
    std::string ip;

} SSDP_SERVER;

typedef std::list<SSDP_SERVER*> SSDPServerList;

class SSDPServer: public WorkerThread
{
public:
    SSDPServer(std::string name);
    ~SSDPServer();

    const char* c_str()
    {
        return (m_name.c_str());
    }

private:
    virtual void Task();
    std::string m_name;
    int m_handle;
    struct sockaddr_in m_local_addr;
    struct sockaddr_in m_server_addr;
    char m_ifaddr[16];
};

class SSDPClient: public WorkerThread
{
public:
    SSDPClient(std::string name);
    ~SSDPClient();
    VC_STATUS SearchDevices(SSDPServerList& list);

    const char* c_str()
    {
        return (m_name.c_str());
    }

private:
    virtual void Task();
    std::string m_name;
    int m_handle;
    struct sockaddr_in m_local_addr;
    struct sockaddr_in m_server_addr;
    Mutex m_mutex;
    ConditionVariable m_cv;
    SSDPServerList m_servers;
};

#endif /* SSDPDISCOVERY_H_ */
