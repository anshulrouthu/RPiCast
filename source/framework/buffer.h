/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * buffer.h
 *
 *  Created on: Jan 28, 2014
 *      Author: anshul
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include "utils.h"

#define VC_BUFFER_MAXSIZE 100*1024 //100 KB of size
class Buffer
{
public:
    Buffer(size_t size = VC_BUFFER_MAXSIZE);
    ~Buffer();
    void* GetData();
    size_t GetSize();
    size_t GetMaxSize();
    int GetSamples();
    VC_STATUS SetSamples(int samples);
    VC_STATUS SetSize(size_t size);
    VC_STATUS Reset();
    VC_STATUS WriteData(void* buf, size_t size);
    BUF_TAG GetTag();
    VC_STATUS SetTag(BUF_TAG tag);

private:
    const char* c_str()
    {
        return ("Buffer");
    }
    void* m_data;
    size_t m_size;
    BUF_TAG m_tag;
    int m_samples;

};

#endif /* BUFFER_H_ */
