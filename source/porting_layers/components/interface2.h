/*
 * interface2.h
 *
 *  Created on: Jul 12, 2014
 *      Author: anshulrouthu
 */

#ifndef INTERFACE2_H_
#define INTERFACE2_H_

class VideoCapture;

void senddata2(unsigned char* data, unsigned int size);

void initialize2(VideoCapture* dev);
void uninitialize2();


#endif /* INTERFACE2_H_ */
