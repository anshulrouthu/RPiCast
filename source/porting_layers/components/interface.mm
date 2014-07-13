
#include "avfoundation.h"
#include "interface.h"
#include "interface2.h"

void senddata(unsigned char* data, unsigned int size)
{
    senddata2(data, size);
}

void initialize()
{
    initializeCapture();
}

void uninitialize()
{
    uninitializeCapture();
}