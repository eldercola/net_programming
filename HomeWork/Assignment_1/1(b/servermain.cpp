#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

/* You will to add includes here */
#include <string.h>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>

// Included to get the support library
#include "calcLib.h"
#include "calcLib.c"

#include "protocol.h"


using namespace std;
//longest length of datagram
#define MAXLENGTH 1024
//the working means the server is hanldling the datagram
#define WORKING 1
//the waiting means the server is now available for clients
#define WAITING 0
//make bzero() works normally
#define bzero(a,b) memset(a, 0, b)
//set a port number
#define MYPORT 5000
/* Needs to be global, to be rechable by callback and main */
int loopCount=0;
int terminate=0;
int id = 0;//id starts from 0
int work = WAITING;// not working at the start
map<int, int> communication_ID;//the map stores the clients' datagrams id and the waiting time
mutex map_lock;//The lock protecting communication_id

/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum){
  // As anybody can call the handler, its good coding to check the signal number that called it.
  if(work == WAITING){// the server is available, just record the time.
    loopCount++;
  }
  else{
    printf("Let me be, I want to sleep.\n");
  }
  //check the communication_id
  //1. lock the map
  map_lock.lock();
  //2. check in turn
  for(map<int, int>::iterator it = communication_ID.begin(); it != communication_ID.end();){
    it->second++;
    if(it->second>=10){
      printf("Client %d waits more than 10s.\n", it->first);
      it = communication_ID.erase(it);
    }
    else ++it;
  }
  map_lock.unlock();

  if(loopCount>20){
    printf("I had enough.\n");
    terminate=1;
  }
  return;
}

int main(int argc, char *argv[]){
  
  /* Do more magic */


  /* 
     Prepare to setup a reoccurring event every 1s. If it_interval, or it_value is omitted, it will be a single alarm 10s after it has been set. 
  */
  struct itimerval alarmTime;
  alarmTime.it_interval.tv_sec=1;
  alarmTime.it_interval.tv_usec=0;
  alarmTime.it_value.tv_sec=1;
  alarmTime.it_value.tv_usec=0;

  /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
  signal(SIGALRM, checkJobbList);
  setitimer(ITIMER_REAL,&alarmTime,NULL); // Start/register the alarm. 
  //create socket
  int servfd, rvsdlen;
  struct sockaddr_in servAddr;
  struct sockaddr_in clitAddr;
  socklen_t address_length;
  address_length = sizeof(clitAddr);
  if((servfd = socket(AF_INET, SOCK_DGRAM, 0))==-1){
    perror("create socket");
    exit(1);
  }
  //initialize servAddr
  bzero(&servAddr, sizeof(servAddr));
  servAddr.sin_family = AF_INET;
  servAddr.sin_port = htons(MYPORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  //bind port & address of server
  if(bind(servfd, (struct sockaddr*)&servAddr, sizeof(servAddr))==-1){
    perror("bind");
    exit(1);
  }
  //prepare for receiving and sending messages
  char *ope;//ope is the string to save the operation message
  char rvsdbuf[MAXLENGTH]={0};
  socklen_t socketLength;
  calcProtocol ptc, respondePtc;
  calcMessage msg;
  //start communicating
  while(rvsdlen = recvfrom(servfd,rvsdbuf, MAXLENGTH, 0, (struct sockaddr*)&clitAddr, address_length)){
    //if receive something error
    if(rvsdlen<0){
      printf("Client error!\n");
      break;
    }
    //receive normally works
    else{
      work = WORKING;
      printf("Server received a message from client.\n");
      uint16_t ver;
      memcpy(&ver, rvsdbuf, sizeof(ver));
      ver = ntohs(ver);//use ntohs to translate network bytes to host bytes
      2021.5.4 23:37
    }
  }

  
  while(terminate==0){
    printf("This is the main loop, %d time.\n",loopCount);
    sleep(1);
    loopCount++;
  }

  printf("done.\n");
  return(0);


  
}
