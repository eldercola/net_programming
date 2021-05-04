#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* You will to add includes here */
#include<math.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>

// Included to get the support library
#include "calcLib.h"
#include "calcLib.c"

// Enable if you want debugging to be printed, see examble below.
// Alternative, pass argument during compilation '-DDEBUG'
#define DEBUG
#define BACKLOG 5

using namespace std;

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int add(int a, int b){return a+b;}
int subtraction(int a,int b){
    if(a-b>0)return a-b;
    return b-a;
}
int multi(int a, int b){return a*b;}
int division(int a,int b)
{
    return (int)a/b;
}
float fadd(float a, float b){return a+b;}
float fsubtraction(float a,float b){
    if(a-b>0)return a-b;
    return b-a;
}
float fmulti(float a, float b){return a*b;}
float fdiv(float a, float b){return a/b;}

int main(int argc, char *argv[]){
  
  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
     Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */
  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);
  char clientAdd[20];
  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter. 
   /* Do magic */
  int listenfd,connfd; // listen on listenfd, new connection on connfd
  int bindResult;
  listenfd = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
  
  struct sockaddr_in myServer;
  struct sockaddr_storage client_address;
  socklen_t sin_size;

  bzero((char*)&myServer,sizeof(myServer));
  myServer.sin_family = AF_INET;
  myServer.sin_port = htons(atoi(Destport));
  myServer.sin_addr.s_addr = inet_addr(Desthost);

  #ifdef DEBUG  
  printf("Host %s, and port %s.\n",Desthost,Destport);
  #endif

  if((bindResult = bind(listenfd, (struct sockaddr* )&myServer, sizeof myServer))==-1)
  {
    perror("bind error");
    exit(1);
  }
  //bind succeed
 if(listen(listenfd, BACKLOG) == -1)
 {
   perror("listen");
   exit(1);
 }
  printf("Server Start\n");
  char sdmsg[1024]={0};
  char rvmsg[1024]={0};
  char *operate;
  int rvlen;
  int longest = sizeof(rvmsg);
  int childCnt = 0;
  while(1){
    sin_size = sizeof(client_address);
    connfd = accept(listenfd, (struct sockaddr *)&client_address, &sin_size);
    if(connfd == -1){
      perror("accept");
      continue;
    }
    inet_ntop(client_address.ss_family,
      get_in_addr((struct sockaddr*)&client_address),
      clientAdd, sizeof clientAdd);
    
    printf("server: Connection %d from %s\n",childCnt, clientAdd);
    printf("server: Sending TEXT TCP 1.0 \n");
		struct sockaddr_in *local_sin=(struct sockaddr_in*)&client_address;
		if (send(connfd, "TEXT TCP 1.0\n\n", 14, 0) == -1){
		  perror("send");
		  close(connfd);
		  continue; //leave loop execution, go back to the while, main accept() loop. 
		}
    while(1){
      double opeResult;
      rvlen = recv(connfd, &rvmsg, longest, 0);
      printf("Child[%d] (%s:%d): recv(%d) .\n", childCnt,clientAdd,ntohs(local_sin->sin_port),rvlen);
      if(rvlen == 0){
        printf("Child [%d] died.\n",childCnt);
		    close(connfd);
		    break;
      }
      printf("%s\n",rvmsg);
      if(strcmp(rvmsg, "OK\n")==0){//server receives ok
        printf("Receive OK from the client.\n");
        initCalcLib();
        operate = randomType();
        if(operate[0] == 'f'){
          float f1,f2;
          f1 = randomFloat();
          f2 = randomFloat();
          while(operate[1] == 'd'&&f2 == 0.0f)f2 = randomFloat();
          sprintf(sdmsg, "%s %8.8g %8.8g\n", operate , f1, f2);
           switch (operate[1])
                {
                    case 'a':
                        opeResult = fadd(f1, f2);
                        break;
                    
                    case 's':
                        opeResult = fsubtraction(f1, f2);
                        break;

                    case 'm':
                        opeResult = fmulti(f1, f2);
                        break;

                    case 'd':
                        opeResult = fdiv(f1, f2);
                        break;
                }
        }
        else{
          int val1,val2;
          val1 = randomInt();
          val2 = randomInt();
          while(operate[0] == 'd'&&val2 == 0.0f)val2 = randomInt();
          sprintf(sdmsg, "%s %d %d\n", operate, val1, val2);
          switch (operate[0])
                {
                    case 'a':
                        opeResult = add(val1, val2) + 0.0f;
                        break;
                    
                    case 's':
                        opeResult = subtraction(val1, val2) + 0.0f;
                        break;

                    case 'm':
                        opeResult = multi(val1, val2) + 0.0f;
                        break;

                    case 'd':
                        opeResult = division(val1, val2) + 0.0f;
                        break;
                }
        }
        printf("send %s to client\n", sdmsg);
        if(send(connfd, &sdmsg, sizeof(sdmsg),0) == -1){
          perror("send operators");
          break;
        }
        rvlen = recv(connfd, &rvmsg, longest, 0);
        if(rvlen == -1){
          perror("receive");
          break;
        }
        printf("local result: %f\n", opeResult);
        printf("Remote result: %s\n", rvmsg);
        double remoteResult;
        sscanf(rvmsg, "%lf", &remoteResult);
        memset(rvmsg, 0, sizeof(rvmsg));
        printf("remoteResult-opeResult = %lf\n",fabs(remoteResult-opeResult));
        if(fabs(remoteResult-opeResult) < 0.00001){
          if(send(connfd, "OK\n", sizeof("OK\n"), 0) == -1){
            perror("send OK");
            break;
          }
        }
        else{
          if(send(connfd, "ERROR\n", sizeof("ERROR\n"), 0) == -1){
            perror("send ERROR");
            break;
          }
        }
      }
      else{
        printf("NOT OK\n");
        break;
      }
    }
    printf("connfd close\n\n");
    close(connfd);
  }
  printf("listenfd close\n");
  close(listenfd);
  return 0;
}
