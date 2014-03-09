/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * mutex.h
 *
 *  Created on: Feb 18, 2014
 *      Author: anshul
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <sys/time.h>
#include "utils.h"
#include "osapi.h"

#define WAIT_FOREVER -1
static int const NSEC_PER_SEC = 1000000000;

/**
 *  A wrapper object for pthread mutex. that allows easy locking and unlocking
 */
class Mutex
{
    friend class ConditionVariable;
public:
    Mutex();
    ~Mutex();
    int Lock();
    int Unlock();
    int TryLock();
private:
    OS_MUTEX m_mutex;

};

/**
 * A wrapper object for pthread condition variables. that allows easy wait and signal event
 */
class ConditionVariable
{
public:
    ConditionVariable(Mutex& mutex);
    ~ConditionVariable();
    int Notify();
    int Wait(int millisconds = WAIT_FOREVER);
private:
    OS_COND m_condition;
    Mutex& m_mutex;
};

/**
 * Class provides automatic mechanism to lock the mutex in constructor and
 * unlock the mutex in destructor, when object is out of scope
 * @param mutex
 */
class AutoMutex
{
public:
    AutoMutex(Mutex* mutex);
    ~AutoMutex();
    const char* c_str()
    {
        return ("AutoMutex");
    }
private:
    VC_STATUS Lock();
    VC_STATUS Unlock();
    Mutex* m_mutex;
    bool m_locked;
};

/**
 * Class provides automatic mechanism to unlock the mutex in constructor and
 * lock the mutex in destructor, when object is out of scope
 * @param mutex
 */
class AutoMutexRelease
{
public:
    AutoMutexRelease(Mutex* mutex);
    ~AutoMutexRelease();
    const char* c_str()
    {
        return ("AutoMutexRelease");
    }
private:
    VC_STATUS Lock();
    VC_STATUS Unlock();
    Mutex* m_mutex;
    bool m_locked;
};

#endif /* MUTEX_H_ */
