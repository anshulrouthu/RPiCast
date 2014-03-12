//============================================================================
// Name        : soeket_server.cpp
// Author      : anshul
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

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
#include "osapi.h"
using namespace std;
void *startServer(void*);
void *readMessage(void*);
char message[1024], in_message[1024];
int flag = 0, rflag = 0;
pthread_t clients[10];
pthread_t rclients[10];
int noClient=0;
int serverSock = 0,clientSock=0;
pthread_t server;
pthread_t readMsg[10];
int main(int argc, char *argv[]) {
	long port;
	cout << "Enter the port number to listen:";
	cin >> port;
	OS_THREAD_CREATE(&server, NULL, startServer, (void*) port);
	while (true) {
		scanf("%s", message);
		if (!strcmp(message, "exit")){
			close (clientSock);
			break;
		}
		//send((SOCKET) client, message, strlen(message), 0);
		write(clientSock, message, strlen(message));
	}
}
void *startServer(void *ptr) {
	long port = (long)ptr;
	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	serverSock = socket(AF_INET, SOCK_STREAM, 0);
	bind(serverSock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	listen(serverSock, 1);
	cout<<"Server is listening at port: "<<port<<endl;
	while (1) {
		clientSock = accept(serverSock, (struct sockaddr*) NULL, NULL);
		cout<<"A new client connection";
		OS_THREAD_CREATE(&readMsg[noClient],NULL,readMessage,(void*)clientSock);
		noClient++;
		//snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
		sleep(1);
		cout<<"Not accepting any more connections";
		break;
	}
	return NULL;
}
void *readMessage(void *client){
	//cout<<"reading started";
	int no = noClient;
	int bytes;
		while(true){
			if((bytes=read((int)client,in_message,1024))>0){
				//cout<<bytes<<"bytes recieved ";
				cout<<"Client:";
				for(int i=0;i<bytes;i++)
					cout<<in_message[i];
				cout<<endl;
			}
		}
	return NULL;
}
