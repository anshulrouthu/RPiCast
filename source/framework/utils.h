/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * utils.h
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <iostream>
#include <fstream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <sstream>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>
#include <list>

#ifndef NULL
#define NULL   ((void *) 0)
#endif

#define NO_OF_CHANNELS 1

void DebugSetLevel(int level);
int kbhit(void);
int getch(void);
std::string ItoString(int id);

/**
 * Debugging code: helps us in easy debugging of the application
 */
class vcDebug
{
public:
    vcDebug(int level);
    void DebugPrint(const char* formatString, ...) __attribute__((__format__(printf, 2, 3)));

private:
    bool isDebugOn();
    int m_level;
};

#define LEVEL_ALWAYS 1
#define LEVEL_ERROR 2
#define LEVEL_MESSAGE 3
#define LEVEL_TRACE 4

#define DBGPRINT(level,msg) vcDebug(level).DebugPrint msg

#define DBG_PRINT(level, format, args...)  DBGPRINT(level, ("%-5d%s() - " format "\n", __LINE__, __FUNCTION__, ##args))

//debug macros for functions that do not implement c_str()

#define DBG_DBG_STATIC(level, format, args...)  DBGPRINT(level, ("%-5d%s() - " format "\n", __LINE__, __FUNCTION__, ##args))
#define DBG_ALL_STATIC(format, args...)  DBG_DBG_STATIC(LEVEL_ALWAYS,  format, ##args)
#define DBG_TRACE_STATIC(format, args...)  DBG_DBG_STATIC(LEVEL_TRACE,   format, ##args)
#define DBG_MSG_STATIC(format, args...)  DBG_DBG_STATIC(LEVEL_MESSAGE,   format, ##args)
#define DBG_ERR_STATIC(format, args...)  DBG_DBG_STATIC(LEVEL_ERROR,  format, ##args)
#define DBG_CHECK_STATIC(condition, fail, msg, args...)  \
do                                      \
{                                       \
    if (condition)                      \
    {                                   \
        DBG_ERR_STATIC(msg,##args);             \
        fail;                           \
    }                                   \
} while (0)

//To use the following debug macros, the class should implement c_str() method

#define DBG_DBG(level, format, args...)  DBGPRINT(level, ("%-5d%s::%s() - " format "\n", __LINE__, c_str(), __FUNCTION__, ##args))
#define DBG_DBG_ERR(level, format, args...)  DBGPRINT(level, ("%-5d%s::%s() - " format " : %s\n", __LINE__, c_str(), __FUNCTION__, ##args, __FILE__))
#define DBG_ERR(format, args...)  DBG_DBG_ERR(LEVEL_ERROR,  format, ##args)
#define DBG_ALL(format, args...)  DBG_DBG(LEVEL_ALWAYS,  format, ##args)
#define DBG_MSG(format, args...)  DBG_DBG(LEVEL_MESSAGE,   format, ##args)
#define DBG_TRACE(format, args...)  DBG_DBG(LEVEL_TRACE,   format, ##args)

#define DBG_CHECK(condition, fail, msg, args...)  \
do                                      \
{                                       \
    if (condition)                      \
    {                                   \
        DBG_ERR(msg,##args);             \
        fail;                           \
    }                                   \
} while (0)

//debugging code end

typedef enum
{

} VC_EVENT;

typedef enum
{
    VC_CMD_START = 0, VC_CMD_STOP
} VC_CMD;

typedef enum
{
    VC_CAPTURE_DEVICE = 0,
    VC_AUDIO_PROCESSOR,
    VC_FLAC_DEVICE,
    VC_CURL_DEVICE,
    VC_TEXT_PROCESSOR,
    VC_COMMAND_PROCESSOR,
    VC_FILESINK_DEVICE,
    VC_FILESRC_DEVICE

} VC_DEVICETYPE;

typedef enum
{
    VC_FAILURE = 0, VC_SUCCESS, VC_NOT_IMPLEMENTED, VC_UNDEFINED
} VC_STATUS;

typedef enum
{
    TAG_NONE = 0, TAG_START, TAG_BREAK, TAG_END, TAG_EOF
} BUF_TAG;

#endif /* UTILS_H_ */
