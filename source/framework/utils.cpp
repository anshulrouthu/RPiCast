/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * utils.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */
#include "utils.h"

static int g_dbglevel __attribute__ ((unused)) = 2;

void DebugSetLevel(int level)
{
    g_dbglevel = level;
    DBG_PRINT(LEVEL_MESSAGE, "Debug Level %d", g_dbglevel);
}

int kbhit(void)
{
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);

    select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
    int c = FD_ISSET(STDIN_FILENO, &rdfs);
    return c;
}

/* reads from keypress, doesn't echo */
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

/**
 * A utility function to convert int to string
 * @param id
 * @return string
 */
std::string ItoString(int id)
{
    std::ostringstream os;
    os << id;
    return (os.str());
}

vcDebug::vcDebug(int level) :
    m_level(level)
{
}

void vcDebug::DebugPrint(const char* formatString, ...)
{
    if (isDebugOn())
    {
        va_list argumentList;
        va_start(argumentList, formatString);
        vprintf(formatString, argumentList);
        va_end(argumentList);
    }

}

bool vcDebug::isDebugOn()
{
    return (m_level <= g_dbglevel);
}

