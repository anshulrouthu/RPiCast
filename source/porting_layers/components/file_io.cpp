/*********************************************************************
 RPiCast ( Screencasting application using RaspberryPi )

 Copyright (C)  Anshul Routhu <anshul.m67@gmail.com>

 All rights reserved.

 This file file_io.cpp is part of RPiCast project

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
 *********************************************************************/

#include "file_io.h"

FileSink::FileSink(std::string name, const char* filename) :
    ADevice(name),
    m_filename(filename)
{
    DBG_MSG("Enter");
}

FileSink::~FileSink()
{
    DBG_MSG("Enter");
}

/**
 * Initialize the file capture device
 */
VC_STATUS FileSink::Initialize()
{
    DBG_MSG("Enter");
    m_input = new InputPort("FileSink Input", this);
    m_file = fopen(m_filename, "wb");
    DBG_CHECK(!m_file, return (VC_FAILURE), "Error(%d): Unable to open file %s", (int)m_file, m_filename);
    return (VC_SUCCESS);
}

/**
 * Initialize the file capture device
 */
VC_STATUS FileSink::Uninitialize()
{
    DBG_MSG("Enter");

    {
        AutoMutex automutex(&m_mutex);
        m_cv.Notify();
    }

    Join();

    fclose(m_file);
    delete m_input;
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
            while (!m_input->IsBufferAvailable() && m_state)
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
    DBG_MSG("Enter");
}

FileSrc::~FileSrc()
{
    DBG_MSG("Enter");
}

VC_STATUS FileSrc::Initialize()
{
    DBG_MSG("Enter");
    m_output = new OutputPort("FileSrc Output", this);
    m_file = fopen(m_filename, "rb");
    DBG_CHECK(!m_file, return (VC_FAILURE), "Error opening file %s", m_filename);
    return (VC_SUCCESS);
}

VC_STATUS FileSrc::Uninitialize()
{
    DBG_MSG("Enter");
    fclose(m_file);
    delete m_output;
    Join();
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
    DBG_MSG("Enter");

    Buffer* buf = m_output->GetBuffer();
    buf->SetTag(TAG_START);
    m_output->PushBuffer(buf);

    while (m_state)
    {
        Buffer* buf = m_output->GetBuffer();
        size_t size = fread(buf->GetData(), 1, buf->GetMaxSize(), m_file);

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
        m_cv.Wait(5);
    }

}
