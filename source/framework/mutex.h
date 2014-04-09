/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file mutex.h is part of RPiCast project

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
