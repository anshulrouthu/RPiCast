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

#define VC_BUFFER_MAXSIZE (1024 * 1024) //1 MB of size
class Buffer
{
public:
    Buffer(size_t size = VC_BUFFER_MAXSIZE);
    virtual ~Buffer();
    virtual void* GetData();
    virtual size_t GetSize();
    virtual size_t GetMaxSize();
    virtual int GetSamples();
    virtual VC_STATUS SetSamples(int samples);
    virtual VC_STATUS SetSize(size_t size);
    virtual VC_STATUS Reset();
    virtual VC_STATUS WriteData(void* buf, size_t size);
    virtual BUF_TAG GetTag();
    virtual VC_STATUS SetTag(BUF_TAG tag);

protected:
    void* m_data;
    size_t m_size;
    BUF_TAG m_tag;

private:
    const char* c_str()
    {
        return ("Buffer");
    }
    int m_samples;

};

#endif /* BUFFER_H_ */
