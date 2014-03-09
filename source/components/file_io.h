/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * file_capture.h
 *
 *  Created on: Feb 8, 2014
 *      Author: anshul
 */

#ifndef FILE_CAPTURE_H_
#define FILE_CAPTURE_H_

#include "apipe.h"
#include "utils.h"
#include "worker.h"

class FileSink: public ADevice
{
public:
    FileSink(std::string name, const char* filename);
    virtual ~FileSink();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno);
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);

private:
    VC_STATUS WriteData();
    FILE* m_file;
    InputPort* m_input;
    OutputPort* m_output;
    const char* m_filename;

};

class FileSrc: public ADevice, public WorkerThread
{
public:
    FileSrc(std::string name, const char* in_file);
    virtual ~FileSrc();

    virtual VC_STATUS Initialize();
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);

private:
    virtual void Task();
    VC_STATUS ReadData();
    FILE* m_file;
    OutputPort* m_output;
    const char* m_filename;

};
#endif /* FILE_CAPTURE_H_ */
