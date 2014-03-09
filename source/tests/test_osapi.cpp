/*
 * test_osapi.cpp
 *
 *  Created on: Mar 8, 2014
 *      Author: anshul
 */

#include "utils.h"
#include "osapi.h"
#include "mutex.h"

Mutex mutex;
ConditionVariable cv(mutex);
bool wakeup = false;

static void* ThreadTest(void* p)
{
    AutoMutex automutex(&mutex);
    DBGPRINT(LEVEL_ALWAYS, ("Mutex held by %s\n", __FUNCTION__));
    while (!wakeup)
    {
        DBGPRINT(LEVEL_ALWAYS, ("%s is sleeping\n", __FUNCTION__));
        cv.Wait();
    }
    return 0;
}

int main(int argc, char* argv[])
{
    DBGPRINT(LEVEL_ALWAYS, ("OSAPI Component Test\n"));

    OS_THREAD_HANDLE handle;

    OS_THREAD_CREATE(&handle, NULL, &ThreadTest, NULL );
    usleep(100000);
    {
        AutoMutex automutex(&mutex);
        DBGPRINT(LEVEL_ALWAYS, ("Mutex held by %s\n", __FUNCTION__));
        wakeup = true;
        DBGPRINT(LEVEL_ALWAYS, ("Wake up the thread\n"));
        cv.Notify();
    }

    OS_THREAD_JOIN(&handle, NULL );

    DBGPRINT(LEVEL_ALWAYS, ("OSAPI Component Test Successful\n"));
    return (0);
}
