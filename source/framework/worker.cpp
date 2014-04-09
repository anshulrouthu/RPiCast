/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file worker.cpp is part of RPiCast project

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

#include "worker.h"

#ifndef WORKERTHREAD_CPP_
#define WORKERTHREAD_CPP_

/**
 * Workerthread Constructor
 */
WorkerThread::WorkerThread() :
    m_state(false)
{
}

/**
 * Workerthread Destructor
 */
WorkerThread::~WorkerThread()
{
}

/**
 * This function creates a thread of calling instance
 */
VC_STATUS WorkerThread::Start()
{
    m_state = true;
    int t = OS_THREAD_CREATE(&m_handle, NULL, &WorkerThread::taskLauncher, (void*) this);
    DBG_CHECK_STATIC(t != 0, return (VC_FAILURE), "Failed to create thread (error: %d)", t);

    return (VC_SUCCESS);

}

/**
 * A static function that initiates the calling object's thread.
 */
void* WorkerThread::taskLauncher(void* p)
{
    WorkerThread* c = static_cast<WorkerThread*>(p);
    c->Task();
    return (NULL);
}

/**
 * Joins the thread
 */
int WorkerThread::Join()
{
    return (OS_THREAD_JOIN(&m_handle, NULL));
}

/**
 * Stops the thread
 */
void WorkerThread::Stop()
{
    m_state = false;
}

#endif /* WORKERTHREAD_CPP_ */
