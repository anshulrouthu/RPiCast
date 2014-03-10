/*
 * avpipe.h
 *
 *  Created on: Mar 9, 2014
 *      Author: anshul
 */

#ifndef AVPIPE_H_
#define AVPIPE_H_

#include "basepipe.h"
#include "video_capture.h"
#include "video_encoder.h"
#include "file_io.h"
#include "socket_io.h"

class AVPipe: public BasePipe
{
public:
    AVPipe(std::string name);
    virtual ~AVPipe() {}
    virtual ADevice* GetDevice(VC_DEVICETYPE devtype, std::string name, const char* filename);
};



#endif /* AVPIPE_H_ */
