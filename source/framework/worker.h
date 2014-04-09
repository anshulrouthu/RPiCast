/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file worker.h is part of RPiCast project

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
