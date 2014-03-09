/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * timer.h
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

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
