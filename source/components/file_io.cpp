/***********************************************************
 voiceCommand

 Copyright (c) 2014 Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This software is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 ***********************************************************/

/*
 * file_capture.cpp
 *
 *  Created on: Feb 8, 2014
 *      Author: anshul
 */
#include "file_io.h"

FileSink::FileSink(std::string name, const char* filename) :
    ADevice(name),
    m_filename(filename)
{
}

FileSink::~FileSink()
{
    fclose(m_file);
    delete m_input;
}

/**
 * Initialize the file capture device
 */
VC_STATUS FileSink::Initialize()
{
    DBG_TRACE("Enter");
    m_input = new InputPort("FileSink Input", this);
    m_file = fopen(m_filename, "wb");
    return (VC_SUCCESS);
}

/**
 * Write data in to a file
 */
VC_STATUS FileSink::WriteData()
{
    DBG_TRACE("Enter");
    while (m_input->IsBufferAvailable())
    {
        Buffer* buf = m_input->GetFilledBuffer();
        if (buf->GetTag() == TAG_NONE)
        {
            fwrite(buf->GetData(), buf->GetSize(), 1, m_file);
        }
        m_input->RecycleBuffer(buf);
    }
    return (VC_SUCCESS);
}

/**
 * Get the input port
 */
InputPort* FileSink::Input(int portno)
{
    DBG_TRACE("Enter");
    return (m_input);
}

/**
 * Get the output port
 */
OutputPort* FileSink::Output(int portno)
{
    DBG_TRACE("Enter");
    return (m_output);
}

/**
 * Notify the device about an event
 */
VC_STATUS FileSink::Notify(VC_EVENT* evt)
{
    DBG_TRACE("Enter");
    AutoMutex automutex(&m_mutex);
    m_cv.Notify();
    return (VC_SUCCESS);
}

/**
 * Send command to the device
 */
VC_STATUS FileSink::SendCommand(VC_CMD cmd)
{
    switch (cmd)
    {
    case VC_CMD_START:
        Start();
        break;
    case VC_CMD_STOP:
        Stop();
        break;
    }
    return (VC_SUCCESS);
}

void FileSink::Task()
{
    DBG_TRACE("Enter");

    while (m_state)
    {
        if (m_input->IsBufferAvailable())
        {
            Buffer* buf = m_input->GetFilledBuffer();
            if (buf->GetTag() == TAG_NONE)
            {
                fwrite(buf->GetData(), buf->GetSize(), 1, m_file);
            }
            m_input->RecycleBuffer(buf);
        }
        else
        {
            AutoMutex automutex(&m_mutex);
            while(!m_input->IsBufferAvailable() && m_state)
            {
                m_cv.Wait();
            }
        }
    }

}

FileSrc::FileSrc(std::string name, const char* in_file) :
    ADevice(name),
    m_filename(in_file)
{
}

FileSrc::~FileSrc()
{
    fclose(m_file);
    delete m_output;
}

VC_STATUS FileSrc::Initialize()
{
    DBG_TRACE("Enter");
    m_output = new OutputPort("FileSrc Output", this);
    m_file = fopen(m_filename, "rb");
    DBG_CHECK(!m_file, return (VC_FAILURE), "Error opening file %s", m_filename);
    return (VC_SUCCESS);
}

OutputPort* FileSrc::Output(int portno)
{
    return (m_output);
}

VC_STATUS FileSrc::SendCommand(VC_CMD cmd)
{
    switch (cmd)
    {
    case VC_CMD_START:
        Start();
        break;
    case VC_CMD_STOP:
        Stop();
        break;
    }
    return (VC_SUCCESS);
}

void FileSrc::Task()
{
    DBG_TRACE("Enter");

    Buffer* buf = m_output->GetBuffer();
    buf->SetTag(TAG_START);
    m_output->PushBuffer(buf);

    while (m_state)
    {
        Buffer* buf = m_output->GetBuffer();
        size_t size = fread(buf->GetData(), 1, 1024, m_file);

        DBG_TRACE("Size of data read %d", size);

        if (size > 0)
        {
            buf->SetSize(size);
            DBG_CHECK(m_output->PushBuffer(buf) != VC_SUCCESS,, "Failed to push buffer");
        }
        else
        {
            DBG_TRACE("End of file reached");
            buf->SetTag(TAG_END);
            m_output->PushBuffer(buf);
            break;
        }
    }

}
