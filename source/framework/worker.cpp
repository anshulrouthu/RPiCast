/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/**
 * @file worker.cpp
 *
 * Main interface for all the thread in the application.
 *
 */
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
