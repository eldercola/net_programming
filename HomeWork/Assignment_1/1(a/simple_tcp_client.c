#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char** argv)
{
    // create struct for address of destination
    struct sockaddr_in serv_addr;
    //store the socket for client
    int clitfd;

    //initialize
    memset(&serv_addr,0,sizeof(struct sockaddr_in));

    //if failed to create socket
    if((clitfd = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)) == -1)
    {
        printf("Failed to create socket.\n");
        return 0;
    }

    //fulfill the address and port.
    //address
    /*
   char myAddress[12] = "13.53.76.30";
   const char* myAdd = &myAddress[0];
    //port
    char myPort[5] = "5000";
    const char* myPo = &myPort[0];
    */
   char delim[]=":";
   char *Desthost=strtok(argv[1],delim);
   char *Destport=strtok(NULL,delim);

    //pass the value of address and port of server to the struct.
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(Desthost);
    serv_addr.sin_port = htons(atoi(Destport));

    //send connect request
    if(connect(clitfd,(struct sockaddr*)& serv_addr,sizeof(serv_addr)) == -1)
    {
        printf("Failed to connect.\n");
        return 0;
    }

    //start communication
    //int timesOfCommunication = 0;
    while(1)
    {
        char sdbuf[1024];
		char rvbuf[1024];
		int rdlen,sdlen,i=0;
		
		rdlen=0;
        int rdcnt = read(clitfd,&rvbuf[rdlen],sizeof(rvbuf));
        if(rdcnt == -1)
        {
            perror(NULL);
            continue;
        }
        rdlen+=rdcnt;
        if(rdlen)
        {
		     rvbuf[rdlen]='\0';
		    printf("(Client)recv : %s\n", rvbuf);
            if(strcmp(rvbuf,"OK\n")==0)break;
        }   
        else 
		{
	        printf("Server has closed ! \n");
		    printf("Client will close...\n");
		break;
		}
        //write
        printf("(Client)send : ");
 
		//while((sdbuf[i] = getchar()) != '\n')i++;// OK\n OK
        if(rvbuf[0]=='T'){
            sdbuf[0]='O';
            sdbuf[1]='K';
            sdbuf[2]='\n';
            //if(i==0)continue; //if the user just send "Enter", the client will not send anything.
            sdlen = write(clitfd,"OK\n",strlen("OK\n"));  //send the content you input.
            printf(" %s\n", sdbuf);
        }
        else{
            if(rvbuf[0]!='f'){//int
                char command[3];
                int value_1, value_2, result = 0;
                sscanf(rvbuf, "%s %d %d\n", command, &value_1, &value_2);
                switch (command[0])
                {
                    case 'a':
                        result = add(value_1, value_2);
                        break;
                    
                    case 's':
                        result = subtraction(value_1, value_2);
                        break;

                    case 'm':
                        result = multi(value_1, value_2);
                        break;

                    case 'd':
                        result = division(value_1, value_2);
                        break;
                }
                sprintf(sdbuf,"%d\n",result);
                sdlen = write(clitfd, sdbuf, strlen(sdbuf));
                printf(" %s\n", sdbuf);
            }
            else{//float
                char command[4];
                float value_1,value_2, result;
                sscanf(rvbuf,"%s %f %f", command, &value_1, &value_2);
                switch (command[1])
                {
                    case 'a':
                        result = fadd(value_1, value_2);
                        break;
                    
                    case 's':
                        result = fsubtraction(value_1, value_2);
                        break;

                    case 'm':
                        result = fmulti(value_1, value_2);
                        break;

                    case 'd':
                        result = fdiv(value_1, value_2);
                        break;
                }
                sprintf(sdbuf,"%8.8g\n",result);
                sdlen = write(clitfd, sdbuf, strlen(sdbuf));
                printf(" %s\n", sdbuf);
            }
        }
    }
    close(clitfd);
}