/***********************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file buffer.h is part of RPiCast project

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
    virtual unsigned char* GetData();
    virtual size_t GetSize();
    virtual size_t GetMaxSize();
    virtual int GetSamples();
    virtual VC_STATUS SetSamples(int samples);
    virtual VC_STATUS SetSize(size_t size);
    virtual VC_STATUS Reset();
    virtual VC_STATUS WriteData(void* buf, size_t size);
    virtual BUF_TAG GetTag();
    virtual VC_STATUS SetTag(BUF_TAG tag);
    virtual const char* c_str()
    {
        return ("Buffer");
    }
    int64_t m_pts;
protected:
    unsigned char* m_data;
    size_t m_size;
    BUF_TAG m_tag;

private:
    int m_samples;

};

#endif /* BUFFER_H_ */
