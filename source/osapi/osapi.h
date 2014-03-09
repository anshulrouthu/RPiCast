/***********************************************************
 voiceCommand

 This file is a part of VoiceCommand application.
 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

#ifndef OSAPI_H_
#define OSAPI_H_

#ifdef __linux
#include "osapi_linux.h"
#elif _WIN64

#elif _WIN32

#elif __APPLE__

#endif

#endif /* OSAPI_H_ */
