#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<sys/time.h>
#include<sys/select.h>
#include<unistd.h>

#include "calcLib.h"
#include"protocol.h"

#define START 0
#define CLAC 1
char* getArith(int opt){
    if(opt == 1 || opt ==5) return (char*)"+";
    else if(opt == 2 || opt == 6) return (char*)"-";
    else if(opt == 3 || opt == 7)  return (char*)"*";
    else return (char*)"/";
}

int main(int argc, char *argv[])
{
    int retransmit = 0;
    int state = 0;// 0 start; 1 return calc result
    //socket
    int clitfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    char delim[]=":";
    char *serverAddPtr=strtok(argv[1],delim);
    char *serverPortPtr=strtok(NULL,delim);
    /*
    char serverAdd[20] = "13.53.76.30";
    char *serverAddPtr = &serverAdd[0];
    char serverPort[5] = "5000";
    char *serverPortPtr = &serverPort[0];
    */
    struct sockaddr_in servadd, clitadd;
    memset(&servadd,0,sizeof(servadd));
    servadd.sin_family = AF_INET;
    servadd.sin_port = htons(atoi(serverPortPtr));
    servadd.sin_addr.s_addr = inet_addr(serverAddPtr);
    socklen_t address_length;

    struct calcMessage sendMessage,returnMessage;
    struct calcProtocol respondeMessage;
    char rvbuf[1024];
    //first calcMessage
    sendMessage.type = htons(2);
    sendMessage.message = htonl(0);
    sendMessage.protocol = htons(17);
    sendMessage.major_version = htons(1);
    sendMessage.minor_version = htons(0);
    // prepare for select
    int maxfdp;
    fd_set fds;
    struct timeval timeout = {2, 0};
    //first send
    
    while(1)
    {
        if(state == START){
            sendto(clitfd,&sendMessage,sizeof(sendMessage),0,(struct sockaddr*)&servadd, sizeof(servadd));
            getsockname(clitfd,(struct sockaddr*)&clitadd,&address_length);
            printf("First calcMessage has been sent to IP:%s PORT:%d\nFrom IP:%s PORT:%d\n  type: %hu\n", 
                inet_ntoa(servadd.sin_addr), ntohs(servadd.sin_port),inet_ntoa(clitadd.sin_addr), ntohs(clitadd.sin_port),ntohs(sendMessage.type));
        }
        FD_ZERO(&fds);//clean up.
        FD_SET(clitfd, &fds);
        maxfdp = clitfd + 1;
        int num = select(maxfdp, &fds, NULL, NULL, &timeout);
        if(num == -1){
            printf("Select Error\n");
            break;
        }
        else if(num == 0){
            //upper bound of retransmitting
            if(retransmit == 2){
                printf("transmit times = 3, up to the bound.\n");
                break;
            }
            //reset the timeval
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;
            printf("Datagram hasn't been sent successfully.\nResend right now.\n\n");
            if(state == CLAC){//state == 1
            //resend the calcProtocol
                sendto(clitfd, &respondeMessage, sizeof(respondeMessage), 0, (struct sockaddr*)&servadd, sizeof(servadd));
            }
            retransmit++;
            continue;
        }
        else if(FD_ISSET(clitfd,&fds)){//get message from clitfd
            retransmit = 0;
            int ret = recvfrom(clitfd, rvbuf, sizeof(rvbuf), 0, NULL, NULL);
            //something wrong in recvfrom
            if(ret == -1)
            {
                printf("Receive an error.\n");
                break;
            }
            //receive calcProtocol
            else if(ret == 50)
            {
                memcpy(&respondeMessage, rvbuf, ret);
                printf("\nFrom Server:\n  type: %hu\n arith: %s\n inValue1: %u\n   inValue2: %u\n flValue1: %f\n   flValue2: %f\n", 
                ntohs(respondeMessage.type),getArith(ntohl(respondeMessage.arith)),ntohl(respondeMessage.inValue1),ntohl(respondeMessage.inValue2),respondeMessage.flValue1,respondeMessage.flValue2);
                switch(ntohl(respondeMessage.arith)){
                    case 1:
                        respondeMessage.inResult = htonl(ntohl(respondeMessage.inValue1)+ntohl(respondeMessage.inValue2));
                        break;
                    case 2:
                        respondeMessage.inResult = htonl(ntohl(respondeMessage.inValue1)-ntohl(respondeMessage.inValue2));
                        break;
                    case 3:
                        respondeMessage.inResult = htonl(ntohl(respondeMessage.inValue1)*ntohl(respondeMessage.inValue2));
                        break;
                    case 4:
                        respondeMessage.inResult = htonl(ntohl(respondeMessage.inValue1)/ntohl(respondeMessage.inValue2));
                        break;
                    case 5:
                        respondeMessage.flResult = respondeMessage.flValue1+respondeMessage.flValue2;
                        break;
                    case 6:
                        respondeMessage.flResult = respondeMessage.flValue1-respondeMessage.flValue2;
                        break;
                    case 7:
                        respondeMessage.flResult = respondeMessage.flValue1*respondeMessage.flValue2;
                        break;
                    case 8:
                        respondeMessage.flResult = respondeMessage.flValue1/respondeMessage.flValue2;
                        break;
                }
                sendto(clitfd, &respondeMessage, sizeof(respondeMessage), 0, (struct sockaddr*)&servadd, sizeof(servadd));
                state = 1;
                printf("New calcProtocol has been sent.\n   inResult: %u\n  flResult: %lf\n", ntohl(respondeMessage.inResult), respondeMessage.flResult);
                continue;
            }
            //receive calcMessage for assuring the answer
            else if(ret == 12)
            {
                memcpy(&returnMessage,rvbuf, ret);
                if(state == 0){
                    printf("Server returns NOT OK.\n");
                    break;
                }
                else{
                    printf("\n");
                    if(ntohl(returnMessage.message)==1)printf("Server returns OK.\n");
                    else if(ntohl(returnMessage.message)==2)printf("Server returns NOT OK.\n");
                    else printf("Server returns N/A.\n");
                    break;
                }
            } 
        }        
    }
    close(clitfd);
    return 0;
}