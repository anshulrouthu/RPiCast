#include "utils.h"
#include "buffer.h"
#include "av_pipe.h"

int main(int argc, char* argv[])
{
    DBGPRINT(LEVEL_ALWAYS, ("Testing SocketIODeviceTest\n"));

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
    ADevice* server = pipe->GetDevice(VC_SOCKET_RECEIVER, "SocketReceiver");
    ADevice* client = pipe->GetDevice(VC_SOCKET_TRANSMITTER, "SocketTransmitter", "127.0.0.1");
    ADevice* fsink = new FileSink("FileIO FileSink", "socket_test.out");
    ADevice* fsrc = new FileSrc("FileIO FileSrc", "socket_test_src.out");

    server->Initialize();
    client->Initialize();
    fsrc->Initialize();
    fsink->Initialize();

    pipe->ConnectDevices(fsrc, client);
    pipe->ConnectDevices(server, fsink);

    server->SendCommand(VC_CMD_START);
    client->SendCommand(VC_CMD_START);
    fsrc->SendCommand(VC_CMD_START);
    fsink->SendCommand(VC_CMD_START);

    while (getch() != 'q')
        ;

    client->SendCommand(VC_CMD_STOP);
    fsrc->SendCommand(VC_CMD_STOP);
    fsink->SendCommand(VC_CMD_STOP);
    server->SendCommand(VC_CMD_STOP);

    pipe->DisconnectDevices(fsrc, client);
    pipe->DisconnectDevices(server, fsink);

    server->Uninitialize();
    client->Uninitialize();
    fsrc->Uninitialize();
    fsink->Uninitialize();

    delete server;
    delete client;
    delete fsrc;
    delete fsink;

    return (0);
}
