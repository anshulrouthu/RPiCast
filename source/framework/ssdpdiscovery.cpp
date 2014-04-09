/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file ssdpdiscovery.cpp is part of RPiCast project

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

#include "ssdpdiscovery.h"

SSDPServer::SSDPServer(std::string name) :
    m_name(name)
{
    DBG_MSG("Enter");
    int reuse = 1;
    int err;
    struct ip_mreq group;
    struct ifaddrs* ifaddr = NULL;

    m_handle = socket(AF_INET, SOCK_DGRAM, 0);
    DBG_CHECK(m_handle < 0, return, "Error: Creating socket");

    err = setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse));
    DBG_CHECK(err < 0,, "Error: Unable set SO_REUSEADDR");

    memset((char *) &m_server_addr, 0, sizeof(m_server_addr));
    m_local_addr.sin_family = AF_INET;
    m_local_addr.sin_addr.s_addr = INADDR_ANY;
    m_local_addr.sin_port = htons(SSDP_PORT);

    err = bind(m_handle, (struct sockaddr *) &m_local_addr, sizeof(m_local_addr));
    DBG_CHECK(err < 0, return, "Error(%d): Unable to bind socket", err);

    group.imr_multiaddr.s_addr = inet_addr(SSDP_GROUP);
    group.imr_interface.s_addr = htonl(INADDR_ANY);

    err = setsockopt(m_handle, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &group, sizeof(group));
    DBG_CHECK(err < 0,, "Error: Unable add socket to group membership");

    getifaddrs(&ifaddr);

    while (ifaddr)
    {
        if ((ifaddr->ifa_addr->sa_family == AF_INET && !strcmp(ifaddr->ifa_name, "wlan0")))
        {
            inet_ntop(AF_INET, &((struct sockaddr_in *) ifaddr->ifa_addr)->sin_addr, m_ifaddr, INET_ADDRSTRLEN);
            DBG_ALL("External IP address %s: %s", ifaddr->ifa_name, m_ifaddr);
            break;
        }
        ifaddr = ifaddr->ifa_next;
    }

    Start();
}

SSDPServer::~SSDPServer()
{
    Stop();
    shutdown(m_handle, SHUT_RDWR);
    close(m_handle);
    Join();
}

void SSDPServer::Task()
{
    while (m_state)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len;
        client_len = sizeof(client_addr);
        char buffer[SSDP_STRING_SIZE];

        size_t bytes_read = recvfrom(m_handle, buffer, SSDP_STRING_SIZE, 0, (struct sockaddr *) &client_addr, &client_len);

        DBG_MSG("Received SSDP discovery request");
        if (bytes_read && !strcmp(buffer, SSDP_DISCOVERY_REQUEST))
        {
            char hostname[1024];
            gethostname(hostname, 1024);
            std::string ssdp_reply = std::string(SSDP_DEVICE_NAME) + " [" + hostname + "]\r\n" + std::string(m_ifaddr);
            sendto(m_handle, ssdp_reply.c_str(), ssdp_reply.length(), 0, (struct sockaddr *) &client_addr, sizeof(client_addr));
        }

    }
}

SSDPClient::SSDPClient(std::string name) :
    m_name(name),
    m_mutex(),
    m_cv(m_mutex)
{
    DBG_ALL("Enter");
    int err;
    struct in_addr localInterface;

    m_server_addr.sin_addr.s_addr = inet_addr(SSDP_GROUP);
    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_port = htons(SSDP_PORT);

    m_handle = socket(AF_INET, SOCK_DGRAM, 0);
    DBG_CHECK(m_handle < 0, return, "Error: Creating socket");

    /* bind any port number */
    m_local_addr.sin_family = AF_INET;
    m_local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_local_addr.sin_port = htons(0);

    err = bind(m_handle, (struct sockaddr *) &m_local_addr, sizeof(m_local_addr));
    DBG_CHECK(err < 0,, "Error: Unable to bind socket");

    localInterface.s_addr = htonl(INADDR_ANY);
    err = setsockopt(m_handle, IPPROTO_IP, IP_MULTICAST_IF, &localInterface, sizeof(localInterface));
    DBG_CHECK(err < 0,, "Error: Unable to set IP_MULTICAST_IF");

    Start();
}

SSDPClient::~SSDPClient()
{
    Stop();
    shutdown(m_handle, SHUT_RDWR);
    close(m_handle);
    Join();

    for (SSDPServerList::iterator it = m_servers.begin(); it != m_servers.end(); it++)
    {
        delete (*it);
        (*it) = NULL;
    }

    m_servers.clear();
}

VC_STATUS SSDPClient::SearchDevices(SSDPServerList& list)
{
    for (SSDPServerList::iterator it = m_servers.begin(); it != m_servers.end(); it++)
    {
        delete (*it);
        (*it) = NULL;
    }
    m_servers.clear();

    size_t bytes = sendto(m_handle, SSDP_DISCOVERY_REQUEST, sizeof(SSDP_DISCOVERY_REQUEST), 0, (struct sockaddr *) &m_server_addr,
        sizeof(m_server_addr));
    DBG_CHECK(bytes != sizeof(SSDP_DISCOVERY_REQUEST),, "Error: Unable send disconnect call");

    AutoMutex automutex(&m_mutex);
    m_cv.Wait(1000);

    list = m_servers;

    return (VC_SUCCESS);
}

void SSDPClient::Task()
{
    while (m_state)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len;
        client_len = sizeof(client_addr);
        char buffer[SSDP_STRING_SIZE];

        size_t bytes_read = recvfrom(m_handle, buffer, SSDP_STRING_SIZE, 0, (struct sockaddr *) &client_addr, &client_len);

        if (bytes_read)
        {
            std::string ssdp_reply = std::string(buffer);
            std::string ip;
            std::string name;

            size_t pos = ssdp_reply.find("\r\n");

            if (pos != std::string::npos)
            {
                SSDP_SERVER* ssdpserver = new SSDP_SERVER();
                ssdpserver->ip = ssdp_reply.substr(pos + 2, ssdp_reply.length());
                ssdpserver->name = ssdp_reply.substr(0, pos);

                bool add = true;

                for (SSDPServerList::iterator it = m_servers.begin(); it != m_servers.end(); it++)
                {
                    if ((*it)->ip == ssdpserver->ip)
                    {
                        add = false;
                        break;
                    }
                }

                if (add)
                {
                    m_servers.push_back(ssdpserver);
                }
                DBGPRINT(LEVEL_TRACE, ("* %s: %s\n", ssdpserver->name.c_str(), ssdpserver->ip.c_str()));
            }
        }
    }
}
