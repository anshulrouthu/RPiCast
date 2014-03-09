/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * timer.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

#include <unistd.h>
#include "timer.h"

/**
 * Timer Constructor
 */
Timer::Timer()
{
}

/**
 * Function starts the timer
 */
void Timer::StartTimer()
{
    gettimeofday(&m_start_time, NULL);
}

/**
 * Function updated the timer variable to current time (resetting the timer)
 */
void Timer::ResetTimer()
{
    gettimeofday(&m_start_time, NULL);
}

/**
 * Function returns the time elapsed since the resettimer or starttimer call
 * @return mtime in ms
 */
long Timer::GetTimePassed()
{
    long mtime, seconds, useconds;

    gettimeofday(&m_end_time, NULL);
    seconds = m_end_time.tv_sec - m_start_time.tv_sec;
    useconds = m_end_time.tv_usec - m_start_time.tv_usec;

    mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;

    return (mtime);
}

/**
 * Function returns the time elapsed since the resettimer or starttimer call
 * @return mtime in ms
 */
long Timer::StopTimer()
{
    long mtime, seconds, useconds;

    gettimeofday(&m_end_time, NULL);
    seconds = m_end_time.tv_sec - m_start_time.tv_sec;
    useconds = m_end_time.tv_usec - m_start_time.tv_usec;

    mtime = ((seconds) * 1000 + useconds / 1000.0) + 0.5;

    return (mtime);
}
