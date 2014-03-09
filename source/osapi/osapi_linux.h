/***********************************************************
 voiceCommand

 This file is a part of VoiceCommand application.
 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

#ifndef OSAPI_LINUX_H_
#define OSAPI_LINUX_H_

#include <pthread.h>
#include <sys/time.h>

typedef pthread_mutex_t OS_MUTEX;
typedef pthread_cond_t OS_COND;
typedef pthread_mutexattr_t OS_MUTEX_ATTR;
typedef pthread_condattr_t OS_COND_ATTR;
typedef pthread_t OS_THREAD_HANDLE;
typedef pthread_attr_t OS_THREAD_ATTR;

/* OS apis for mutex */
int OS_MUTEX_INIT(OS_MUTEX* mutex, OS_MUTEX_ATTR* attr);
int OS_MUTEX_DESTROY(OS_MUTEX* mutex);
int OS_MUTEX_LOCK(OS_MUTEX* mutex);
int OS_MUTEX_UNLOCK(OS_MUTEX* mutex);
int OS_MUTEX_TRYLOCK(OS_MUTEX* mutex);

/* OS apis for condition variables */
int OS_COND_INIT(OS_COND* cond, OS_COND_ATTR* attr);
int OS_COND_DESTROY(OS_COND* cond);
int OS_COND_WAIT(OS_COND* cond, OS_MUTEX* mutex);
int OS_COND_TIMEDWAIT(OS_COND* cond, OS_MUTEX* mutex, struct timespec* abstime);
int OS_COND_SIGNAL(OS_COND* cond);

/* OS thread apis */
int OS_THREAD_CREATE(OS_THREAD_HANDLE* handle, OS_THREAD_ATTR* attr, void *(*start_routine)(void *), void* appdata);
int OS_THREAD_JOIN(OS_THREAD_HANDLE* handle, void** attr);

#endif /* OSAPI_LINUX_H_ */
