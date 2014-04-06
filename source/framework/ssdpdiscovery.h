/*
 * discovery.h
 *
 *  Created on: Apr 6, 2014
 *      Author: anshul
 */

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

#define SSDP_PORT 54321
#define SSDP_GROUP "225.1.1.1"
#define SSDP_STRING_SIZE 1024
#define SSDP_INFO_DISCOVERY_STRING "discover.rpicast.device.info"
#define SSDP_DEVICE_NAME "RpiCast Server"

typedef struct ssdp_server
{
    std::string name;
    std::string ip;

}SSDP_SERVER;

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
