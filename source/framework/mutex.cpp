/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * mutex.cpp
 *
 *  Created on: Feb 18, 2014
 *      Author: anshul
 */

#include "mutex.h"

/**
 * mutex Constructor
 */
Mutex::Mutex()
{
    OS_MUTEX_INIT(&m_mutex, NULL);
}

/**
 * mutex Destructor
 */
Mutex::~Mutex()
{
    OS_MUTEX_DESTROY(&m_mutex);
}

/**
 * Locks the mutex
 * @return 0 on success or error code
 */
int Mutex::Lock()
{
    return (OS_MUTEX_LOCK(&m_mutex));
}

/**
 * Unlocks the mutex
 * @return 0 on success or error code
 */
int Mutex::Unlock()
{
    return (OS_MUTEX_UNLOCK(&m_mutex));
}

/**
 * Tries to lock the mutex, returns when the mutex is already locked
 * @return 0 if lock is acquired
 */
int Mutex::TryLock()
{
    return (OS_MUTEX_TRYLOCK(&m_mutex));
}

/**
 * Conditionvariable Constructor
 * @param [in] mutex
 */
ConditionVariable::ConditionVariable(Mutex& mutex) :
    m_mutex(mutex)
{
    OS_COND_INIT(&m_condition, NULL);
}

/**
 * Conditionvariable Destructor
 */
ConditionVariable::~ConditionVariable()
{
    OS_COND_DESTROY(&m_condition);
}

/**
 * Waits for the condition to signal or till timeout
 * @param[in] milliseconds timeout in ms (If no value is provided waits till the signal)
 * @return 0 on success or error code
 */
int ConditionVariable::Wait(int milliseconds)
{
    if (milliseconds == WAIT_FOREVER)
    {
        return (OS_COND_WAIT(&m_condition, &m_mutex.m_mutex));
    }
    else
    {
        struct timeval tv;
        struct timespec abstime;

        /* get the current time */
        gettimeofday(&tv, NULL);

        /* set our timeout */
        abstime.tv_sec = tv.tv_sec + (milliseconds / 1000);
        abstime.tv_nsec = (tv.tv_usec + ((milliseconds % 1000) * 1000)) * 1000;
        while (abstime.tv_nsec >= NSEC_PER_SEC)
        {
            abstime.tv_sec += 1;
            abstime.tv_nsec -= NSEC_PER_SEC;
        }

        return (OS_COND_TIMEDWAIT(&m_condition, &m_mutex.m_mutex, &abstime));
    }
}

/**
 * Signals the Conditionvariable that is on wait
 * @return 0 on success or error code
 */
int ConditionVariable::Notify()
{
    return (OS_COND_SIGNAL(&m_condition));
}

/**
 * Constructor for Automutex
 */
AutoMutex::AutoMutex(Mutex* mutex) :
    m_mutex(mutex),
    m_locked(false)
{
    Lock();
}

/**
 * Destructor Automutex
 */
AutoMutex::~AutoMutex()
{
    Unlock();
}

/**
 * Lock the mutex, this is called in the constructor of this class
 */
VC_STATUS AutoMutex::Lock()
{
    if (m_mutex && !m_locked)
    {
        int err = m_mutex->Lock();
        DBG_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Locking Mutex ", err);
        m_locked = true;
    }
    return (VC_SUCCESS);
}

/**
 * Unlock the mutex, this is called from destructor of the class
 */
VC_STATUS AutoMutex::Unlock()
{
    if (m_mutex && m_locked)
    {
        int err = m_mutex->Unlock();
        DBG_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Unlocking Mutex ", err);
        m_locked = false;
    }
    return (VC_SUCCESS);
}

/**
 * Constructor for Automutexrelease
 */
AutoMutexRelease::AutoMutexRelease(Mutex* mutex) :
    m_mutex(mutex),
    m_locked(true)
{
    Unlock();
}

/**
 * Destructor for Automutexrelease
 */
AutoMutexRelease::~AutoMutexRelease()
{
    Lock();
}

/**
 * Locks the mutex, this is called from destructor
 */
VC_STATUS AutoMutexRelease::Lock()
{
    if (m_mutex && !m_locked)
    {
        int err = m_mutex->Lock();
        DBG_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Locking Mutex ", err);
        m_locked = true;
    }
    return (VC_SUCCESS);
}

/**
 * Unlocks the mutex, this is called from constructor
 */
VC_STATUS AutoMutexRelease::Unlock()
{
    if (m_mutex && m_locked)
    {
        int err = m_mutex->Unlock();
        DBG_CHECK(err != 0, return (VC_FAILURE), "Error(%d): Unlocking Mutex ", err);
        m_locked = false;
    }
    return (VC_SUCCESS);
}

