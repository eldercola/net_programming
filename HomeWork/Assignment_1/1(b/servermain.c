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
#include <math.h>

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
int Ter=0;
int id = 0;//id starts from 0
int work = WAITING;// not working at the start
map<int, int> communication_ID;//the map stores the clients' datagrams id and the waiting time
mutex map_lock;//The lock protecting communication_id

//This function is used to get the result of a calc protocol.
void getResult(calcProtocol* ptc){
  switch(ntohl(ptc->arith)){
          case 1:
            ptc->inResult = htonl(ntohl(ptc->inValue1) + ntohl(ptc->inValue2));
            break;
          case 2:
            ptc->inResult = ntohl(ptc->inValue1) - ntohl(ptc->inValue2);
            if(ptc->inResult<0)ptc->inResult = 0 - ptc->inResult;
            ptc->inResult = htonl(ptc->inResult);
            break;
          case 3:
            ptc->inResult = htonl(ntohl(ptc->inValue1) * ntohl(ptc->inValue2));
            break;
          case 4:
            ptc->inResult = htonl(ntohl(ptc->inValue1) / ntohl(ptc->inValue2));
            break;
          case 5:
            ptc->flResult = ptc->flValue1 + ptc->flValue2;
            break;
          case 6:
            ptc->flResult = fabs(ptc->flValue1 - ptc->flValue2);
            break;
          case 7:
            ptc->flResult = ptc->flValue1 * ptc->flValue2;
            break;
          case 8:
            ptc->flResult = ptc->flValue1 / ptc->flValue2;
            break;
  }
  return;
}

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
    Ter=1;
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
  //register id stores current id number.
  int register_id;
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
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
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
  while(rvsdlen = recvfrom(servfd,rvsdbuf, MAXLENGTH, 0, (struct sockaddr*)&clitAddr, &address_length)){
    //if receive something error
    if(rvsdlen<0){
      printf("Client error!\n");
      break;
    }
    //receive normally works
    else{
      work = WORKING;
      if(rvsdlen == sizeof(msg)){
        getpeername(servfd, (struct sockaddr*)&clitAddr, &address_length);
        printf("\nclient: IP:%s PORT:%d\n",inet_ntoa(clitAddr.sin_addr), ntohs(clitAddr.sin_port));
        //receive an clac message, copy it to the msg.
        printf("Server received a calcMessage from client.\n");
        memcpy(&msg, rvsdbuf, rvsdlen);
        if(ntohs(msg.type) == 22 && ntohl(msg.message)== 0){
          register_id = id;
        }
        else{
          work = WAITING;
          loopCount = 0;
          continue;
        }
        printf("Now generate a client id %d.\n", register_id);
        //put register id into communication_id map
        communication_ID[register_id] = 0;
        //generate clacProtocol
        initCalcLib();
        loopCount = 0;
        ope = randomType();
        if(ope[0] == 102){
          ptc.arith = htonl(rand()%4 + 5);
          ptc.flValue1 = randomFloat();
          ptc.flValue2 = randomFloat();
          ptc.inValue1 = htonl(0);
          ptc.inValue2 = htonl(0);
          ptc.inResult = htonl(0);
          ptc.flResult = 0.0f;
        }
        else{
          ptc.arith = htonl(1+rand()%4);
          ptc.inValue1 = htonl(randomInt());
          ptc.inValue2 = htonl(randomInt());
          ptc.flValue1 = 0.0f;
          ptc.flValue2 = 0.0f;
          ptc.inResult = htonl(0);
          ptc.flResult = 0.0f;
        }
        ptc.major_version = htons(1);
        ptc.minor_version = htons(0);
        ptc.type = htons(1);
        ptc.id = htonl(id++);
        //send to client
        sendto(servfd, (char*)&ptc,sizeof(ptc),0,(struct sockaddr*)&clitAddr,address_length);
        printf("Server has generated a clacProtocol and sent to client.\nWait for a response.\n");
      }
      else if(rvsdlen == sizeof(respondePtc)){
        printf("Get a responde from a client.\n");
        memcpy(&respondePtc, rvsdbuf, sizeof(respondePtc));
        register_id = ntohl(respondePtc.id);
        if(communication_ID.count(register_id) == 0){
          //the client is out of the map because its message is out of time.
          printf("This clinet has been deleted.\n");
          work = WAITING;
          loopCount = 0;
          continue;
        }
        //receive a responde normally
        communication_ID[register_id] = 0;//reset the time.
        getResult(&ptc);//obtain the result of calcProtocol
        //check the result from client
        if((ntohl(ptc.arith)<=4 && (ntohl(ptc.inResult) == ntohl(respondePtc.inResult)))||
        (ntohl(ptc.arith)>4 && (ptc.flResult == respondePtc.flResult))){
          msg.type = htons(2);
          msg.message = htonl(1);
          msg.major_version = htons(1);
          msg.minor_version = htons(0);
          msg.protocol = htons(17);
          sendto(servfd, (char *)&msg, sizeof(calcMessage), 0, (struct sockaddr *)&clitAddr, address_length);
          printf("Succeeded!\n");
        }
        else{
          msg.type = htons(2);
          msg.message = htonl(2);
          msg.major_version = htons(1);
          msg.minor_version = htons(0);
          msg.protocol = htons(17);
          sendto(servfd, (char *)&msg, sizeof(calcMessage), 0, (struct sockaddr *)&clitAddr, address_length);
          printf("Failed!\n");
        }
        //erase the register id
        map_lock.lock();
        communication_ID.erase(register_id);
        map_lock.unlock();
        printf("A client has finished!\n\n");
        work = WAITING;
        loopCount = 0;
      }
      //can't handle this type of message
      else{
        msg.type = htons(2);
        msg.message = htonl(0);
        msg.major_version = htons(1);
        msg.minor_version = htons(0);
        msg.protocol = htons(17);
        sendto(servfd, (char *)&msg, sizeof(calcMessage), 0, (struct sockaddr *)&clitAddr, address_length);
        printf("Rejected!\n");
        work = WAITING;
        loopCount = 0;
      }
    }
    work = WAITING;
    loopCount = 0;
  }
  while(Ter==0){
    printf("This is the main loop, %d time.\n",loopCount);
    sleep(1);
    loopCount++;
  }
  printf("done.\n");
  return 0;
}
