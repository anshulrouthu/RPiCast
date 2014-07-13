/*
 * interface2.cpp
 *
 *  Created on: Jul 12, 2014
 *      Author: anshulrouthu
 */



#include "interface2.h"
#include "video_capture.h"
#include "interface.h"

static VideoCapture* capture_device = NULL;

void initialize2(VideoCapture* dev)
{
    capture_device = dev;
    initialize();
}


void senddata2(unsigned char* data, unsigned int size)
{
    capture_device->senddata(data, size);
}


void uninitialize2()
{
    uninitialize();
}
