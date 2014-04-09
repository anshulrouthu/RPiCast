/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file timer.h is part of RPiCast project

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

#ifndef TIMER_H_
#define TIMER_H_

#include<sys/time.h>
#include<stdio.h>

/**
 * A simple timer object. that can be used for timer based operations
 */
class Timer
{
public:
    Timer();
    ~Timer()
    {
    }
    void StartTimer();
    void ResetTimer();
    long GetTimePassed();
    long StopTimer();

private:
    const char* c_str()
    {
        return ("Timer");
    }
    timeval m_start_time;
    timeval m_end_time;
};

#endif /* TIMER_H_ */
