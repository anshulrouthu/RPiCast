/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * worker.h
 *
 *  Created on: Oct 4, 2013
 *      Author: anshul
 */

/**
 * @file worker.h
 *
 * Main interface for all the threads, mutexs and condition variables in the application.
 *
 */
#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include "osapi.h"
#include <sys/time.h>
#include "utils.h"

/**
 * An interface for all the threads in the application.
 */
class WorkerThread
{
public:
    WorkerThread();
    virtual ~WorkerThread();
    VC_STATUS Start();
    int Join();
    void Stop();
private:

    virtual void Task()=0;
    static void* taskLauncher(void* args);
    OS_THREAD_HANDLE m_handle;

protected:
    bool m_state;

};

#endif /* WORKER_THREAD_H_ */
