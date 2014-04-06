/*
 * discovery.cpp
 *
 *  Created on: Apr 6, 2014
 *      Author: anshul
 */

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
    group.imr_interface.s_addr = htonl(INADDR_ANY );

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

        if (bytes_read && !strcmp(buffer, SSDP_INFO_DISCOVERY_STRING))
        {
            DBG_MSG("Receiver SSDP discovery request");
            std::string ssdp_reply = std::string(SSDP_DEVICE_NAME) + "\r\n" + std::string(m_ifaddr);
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
    m_local_addr.sin_addr.s_addr = htonl(INADDR_ANY );
    m_local_addr.sin_port = htons(0);

    err = bind(m_handle, (struct sockaddr *) &m_local_addr, sizeof(m_local_addr));
    DBG_CHECK(err < 0,, "Error: Unable to bind socket");

    localInterface.s_addr = htonl(INADDR_ANY );
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
}

VC_STATUS SSDPClient::SearchDevices(SSDPServerList& list)
{
    size_t bytes = sendto(m_handle, SSDP_INFO_DISCOVERY_STRING, sizeof(SSDP_INFO_DISCOVERY_STRING), 0, (struct sockaddr *) &m_server_addr, sizeof(m_server_addr));
    DBG_CHECK(bytes != sizeof(SSDP_INFO_DISCOVERY_STRING),, "Error: Unable send disconnect call");

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
                m_servers.push_back(ssdpserver);
                DBGPRINT(LEVEL_MESSAGE, ("* %s: %s\n", ssdpserver->name.c_str(), ssdpserver->ip.c_str()));
            }
        }
    }
}
