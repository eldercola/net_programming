#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    char myAddress[20];
    char* myAdd = &myAddress[0];
    char myPort[20];
    char* myPortPtr = &myPort[0];
    sscanf(argv[1],"%[^:]:%[^:]\n",myAdd,myPort);

    int servfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in myServer;
    bzero((char*)&myServer,sizeof(myServer));
    myServer.sin_family = AF_INET;
    myServer.sin_port = htons(atoi(myPortPtr));
    myServer.sin_addr.s_addr = inet_addr(myAdd);
    printf("IP: %s\nPORT: %s\n",myAdd,myPortPtr);
    return 0;
}