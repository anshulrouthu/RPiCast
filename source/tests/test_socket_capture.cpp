/*
 * test_socket_capture.cpp
 *
 *  Created on: Mar 10, 2014
 *      Author: anshul
 */

#include "utils.h"
#include "buffer.h"
#include "av_pipe.h"

int main(int argc, char* argv[])
{
    DBGPRINT(LEVEL_ALWAYS, ("Testing Capture Over Network\n"));

    int c;
    while ((c = getopt(argc, argv, "sf?l:d:t:")) != -1)
    {
        switch (c)
        {
        case 'd':
            DebugSetLevel(strtol(optarg, NULL, 10));
            break;
        default:
            break;
        }
    }

    BasePipe* pipe = new AVPipe("AvPipe");
    ADevice* capture = pipe->GetDevice(VC_CAPTURE_DEVICE, "VIdCapture");
    ADevice* encoder = pipe->GetDevice(VC_VIDEO_ENCODER,"VidEncoder");
    ADevice* server = pipe->GetDevice(VC_SOCKET_RECEIVER, "SocketReceiver");
    ADevice* client = pipe->GetDevice(VC_SOCKET_TRANSMITTER, "SocketTransmitter","127.0.0.1");
    ADevice* fsink = new FileSink("FileIO FileSink", "socket_video.out");

    capture->Initialize();
    encoder->Initialize();
    client->Initialize();
    server->Initialize();
    fsink->Initialize();

    pipe->ConnectDevices(capture, encoder);
    pipe->ConnectDevices(encoder,client);
    pipe->ConnectDevices(server, fsink);

    capture->SendCommand(VC_CMD_START);
    encoder->SendCommand(VC_CMD_START);
    server->SendCommand(VC_CMD_START);
    client->SendCommand(VC_CMD_START);
    fsink->SendCommand(VC_CMD_START);

    while (getch() != 'q');

    capture->SendCommand(VC_CMD_STOP);
    encoder->SendCommand(VC_CMD_STOP);
    client->SendCommand(VC_CMD_STOP);
    server->SendCommand(VC_CMD_STOP);
    fsink->SendCommand(VC_CMD_STOP);

    pipe->DisconnectDevices(capture, encoder);
    pipe->DisconnectDevices(encoder,client);
    pipe->DisconnectDevices(server, fsink);

    capture->Uninitialize();
    encoder->Uninitialize();
    client->Uninitialize();
    server->Uninitialize();
    fsink->Uninitialize();

    delete capture;
    delete encoder;
    delete server;
    delete client;
    delete fsink;

    DBGPRINT(LEVEL_ALWAYS, ("Testing Capture Over Network Successful\n"));
    return (0);
}



