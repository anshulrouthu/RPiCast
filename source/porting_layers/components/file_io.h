/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file file_io.h is part of RPiCast project

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

#ifndef FILE_CAPTURE_H_
#define FILE_CAPTURE_H_

#include "basepipe.h"
#include "utils.h"
#include "worker.h"

class FileSink: public ADevice, public WorkerThread
{
public:
    FileSink(std::string name, const char* filename);
    virtual ~FileSink();

    virtual VC_STATUS Initialize();
    virtual VC_STATUS Uninitialize();
    virtual VC_STATUS Notify(VC_EVENT* evt);
    virtual InputPort* Input(int portno);
    virtual OutputPort* Output(int portno);
    virtual VC_STATUS SendCommand(VC_CMD cmd);

private:
    virtual void Task();
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
    virtual VC_STATUS Uninitialize();
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
