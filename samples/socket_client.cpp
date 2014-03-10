#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include<iostream>
#include<pthread.h>
using namespace std;
void* readData(void*);
void* sendData(void*);
char message[1024];
int clientSock;
int outflag = 0;
int startClient(char*,int);
int main() {
    long port;
    char server_name[1024];
    cout<<"Enter the server adress:";
    scanf("%s",server_name);
    cout<<"Enter the port Number:";
    cin>>port;
    if(startClient(server_name,port)) cout<<"Connection Success"<<endl;
     while(true){
         scanf("%s",message);
         if(!strcmp(message,"exit")) {
             close (clientSock);
             break;
         }
         write((int)clientSock,message,strlen(message));
     }
     return 0;

}
int startClient(char *ptr,int port) {
    int addr;
    char *server_name = ptr ;
    sockaddr_in server;

    clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&server, '0', sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if(inet_pton(AF_INET, server_name, &server.sin_addr)<=0)
        {
            printf("\n inet_pton error occured\n");
            return 1;
        }
    if (connect(clientSock, (struct sockaddr*) &server, sizeof(server))) {
        close(clientSock);
        cout << "Error:3";cout<<"Server Listening on Port:"<<port<<endl;
        exit(0);
    }
    pthread_t read;
    pthread_create(&read,NULL,readData,(void*)clientSock);
    //pthread_create(&send,NULL,sendData,(void*)client);

    return 1;
}

void* readData(void *ptr){

    char in_message[1024];
    int bytes;
    while(true){
        if((bytes=read((int)ptr,in_message,1024))>0){
            //cout<<bytes<<"bytes recieved ";
            cout<<"Server:";
            for(int i=0;i<bytes;i++)
                cout<<in_message[i];
            cout<<endl;
        }
    }
    close((int)ptr);
    return NULL;

}
void* sendData(void* ptr){
    while(1){
        if(outflag){
            write((int)ptr,message,strlen(message));
            outflag=0;
        }
    }
    close((int)ptr);
    return NULL;
}
