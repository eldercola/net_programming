#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <exception>
#include <map>
#include <mutex>
#include <calcLib.h>
#include "protocol.h"

using namespace std;

#define PORT 1234
#define MAXDATASIZE 100

/* You will to add includes here */

// Included to get the support library

// 创建互斥锁
std::mutex mtx;
/* Needs to be global, to be rechable by callback and main */
int loopCount = 0;     // 记录有多久没有收到客户端的报文了
int id = 486;          // sessoin id
int flag = 0;          // 是否在处理客户端报文flag
map<int, int> session; // 记录对各个session id客户端的等待时间

/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobList(int signum)
{
  // 当前没有报文正在处理，记录时间
  if (flag == 0)
  {
    loopCount++;
  }

  //修改session，上互斥锁
  mtx.lock();
  //遍历session
  for (auto it = session.begin(); it != session.end();)
  {
    // 等待时间 ++
    it->second++;
    // 判断这个客户端是否超时, 超时则从session中删除
    if (it->second >= 10)
    {
      it = session.erase(it);
      printf("Client timeout\n");
    }
    else
      ++it;
  }
  // 释放互斥锁
  mtx.unlock();

  // 10s没收到报文了，抱怨一下 (一个服务器的时候我孤独啊)
  if (loopCount >= 10)
  {
    printf("Wait for client`s message.\n");
    loopCount = 0;
  }
  return;
}

int main(int argc, char *argv[])
{
  // 初始化定时器，每1s给signal发送一个信号，执行checkJobList函数
  struct itimerval alarmTime;
  alarmTime.it_interval.tv_sec = 1;
  alarmTime.it_interval.tv_usec = 0;
  alarmTime.it_value.tv_sec = 1;
  alarmTime.it_value.tv_usec = 0;

  /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
  signal(SIGALRM, checkJobList);
  setitimer(ITIMER_REAL, &alarmTime, NULL); // Start/register the alarm.

  // 初始化参数
  int sockfd;
  struct sockaddr_in server;
  struct sockaddr_in client;
  socklen_t addrlen;
  int num;
  calcMessage message;
  calcProtocol protocol;
  calcProtocol protocolRes;
  char *ptr;
  int session_id;
  char buf[MAXDATASIZE];
  addrlen = sizeof(client);

  // 创建socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("Creatingsocket failed.\n");
    exit(1);
  }

  // 初始化服务器信息
  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  // 服务器绑定本地地址/端口
  if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
  {
    perror("Bind()error.\n");
    exit(1);
  }

  // 一直等待接收来自客户端的请求
  while (num = recvfrom(sockfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&client, &addrlen))
  {
    if (num < 0)
    {
      printf("Bad receive :(\n");
      break;
    }
    else
    {
      // 开始营业
      flag = 1;
      printf("Get a message form client\n");
      // 取报文头部，判断报文类别（version为自定义参数，详情见protocol.h）
      uint16_t version;
      memcpy(&version, buf, sizeof(version));
      version = ntohs(version);
      if (version == 2 && num == sizeof(calcMessage))
      {
        printf("The message type is calcMessage\n");
        // copy报文内容到message对象
        memcpy(&message, buf, num);
        if (ntohs(message.type) == 22 && ntohl(message.message) == 0)
        {
          // 创建一个session id
          session_id = id;
        }
        else
        {
          continue;
          flag = 0;
          loopCount = 0;
        }
        printf("Generate an id %d\n", id);
        // 将session id 放入session
        session.insert(make_pair(session_id, 0));
        // 初始化那个随机生成数组的类, 开始初始化protocol对象
        initCalcLib();
        loopCount = 0;
        ptr = randomType();
        if (ptr[0] == 'f')
        {
          protocol.arith = htonl(5 + rand() % 4);
          protocol.flValue1 = htonl(randomFloat());
          protocol.flValue2 = htonl(randomFloat());
        }
        else
        {
          protocol.arith = htonl(1 + rand() % 4);
          protocol.inValue1 = htonl(randomInt());
          protocol.inValue2 = htonl(randomInt());
        }
        protocol.version = htons(1);
        protocol.type = htons(1);
        protocol.major_version = htons(1);
        protocol.minor_version = htons(0);
        protocol.id = htonl(id);
        id++;
        printf("Send protocol to the client\n");
        sendto(sockfd, (char *)&protocol, sizeof(calcProtocol), 0, (struct sockaddr *)&client, addrlen);
        printf("Wait for client response\n");
      }
      else if (version == 1 && num == sizeof(calcProtocol))
      {
        printf("The message type is calcProtocol\n");
        memcpy(&protocolRes, buf, num);
        printf("Get a protocal result\n");
        // 读取来自客户端的session id
        session_id = htonl(protocolRes.id);
        // 判断这个客户端是否连接超时，超时就不理他了
        if (session.count(session_id) == 0)
        {
          printf("The task has been abandoned\n");
          flag = 0;
          loopCount = 0;
          continue;
        }
        // 收到信息 耐心恢复至0
        session[session_id] = 0;
        switch (ntohl(protocol.arith))
        {
        case 1:
          protocol.inResult = htonl(ntohl(protocol.inValue1) + ntohl(protocol.inValue1));
          break;
        case 2:
          protocol.inResult = htonl(ntohl(protocol.inValue1) - ntohl(protocol.inValue1));
          break;
        case 3:
          protocol.inResult = htonl(ntohl(protocol.inValue1) * ntohl(protocol.inValue1));
          break;
        case 4:
          protocol.inResult = htonl(ntohl(protocol.inValue1) / ntohl(protocol.inValue1));
          break;
        case 5:
          protocol.flResult = htonl(ntohl(protocol.flValue1) + ntohl(protocol.flValue2));
          break;
        case 6:
          protocol.flResult = htonl(ntohl(protocol.flValue1) - ntohl(protocol.flValue2));
          break;
        case 7:
          protocol.flResult = htonl(ntohl(protocol.flValue1) * ntohl(protocol.flValue2));
          break;
        case 8:
          protocol.flResult = htonl(ntohl(protocol.flValue1) / ntohl(protocol.flValue2));
          break;
        }
        // 开始计算客户端算的对不对，并发送判断结果到客户端
        printf("Start checking");
        if ((ntohl(protocolRes.arith) <= 4 && ntohl(protocolRes.inResult) == ntohl(protocol.inResult)) ||
            (ntohl(protocolRes.arith) > 4 && ntohl(protocolRes.flResult) == ntohl(protocol.flResult)))
        {
          message.type = htons(2);
          message.message = htonl(1);
          message.major_version = htons(1);
          message.minor_version = htons(0);
          message.protocol = htons(17);
          sendto(sockfd, (char *)&message, sizeof(calcMessage), 0, (struct sockaddr *)&client, addrlen);
          printf("Task success\n");
        }
        else
        {
          message.type = htons(2);
          message.message = htonl(2);
          message.major_version = htons(1);
          message.minor_version = htons(0);
          message.protocol = htons(17);
          sendto(sockfd, (char *)&message, sizeof(calcMessage), 0, (struct sockaddr *)&client, addrlen);
          printf("Task fail\n");
        }
        // 处理完毕，将客户端session id从session中抹去 上互斥锁
        mtx.lock();
        session.erase(session_id);
        // 释放互斥锁
        mtx.unlock();
        printf("One task finished :)\n\n");
        flag = 0;
        loopCount = 0;
      }
      // 这不是我想要的报文
      else
      {
        message.type = htons(2);
        message.message = htonl(2);
        message.major_version = htons(1);
        message.minor_version = htons(0);
        message.protocol = htons(17);
        // 发送错误信息message到客户端
        sendto(sockfd, (char *)&message, sizeof(calcMessage), 0, (struct sockaddr *)&client, addrlen);
        printf("Error response type :(\n");
        flag = 0;
        loopCount = 0;
      }
    }
    flag = 0;
    loopCount = 0;
  }
}