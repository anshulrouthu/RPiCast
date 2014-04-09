/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file timer.cpp is part of RPiCast project

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
