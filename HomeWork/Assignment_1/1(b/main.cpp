#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include the calcLib header file, using <> as its a library and not just a object file we link.  */
#include <calcLib.h>


#include "protocol.h"


/* 
   The aim with this code, is to give you examples HOW to grab a random arithmetic operator, matching values and perform a 
   reference calculation, so that you can eventually compare what came from the other side. 
   It also shows how to decode a string into different variables. 

   You are free to reuse relevant parts, as the assignment is ABOUT the communication, which is omitted from the example. 

*/



/* Std start to main, argc holds the number of arguments provided to the executable, and *argv[] an 
   array of strings/chars with the arguments (as strings). 
*/
int main(int argc, char *argv[]){

  /* Initialize the library, this is needed for this library. */
  initCalcLib();
  char *ptr;
  ptr=randomType(); // Get a random arithemtic operator. 

  double f1,f2,fresult;
  int i1,i2,iresult;
  /*
  printf("ptr = %p, \t", ptr );
  printf("string = %s, \n", ptr );
  */

  /* Act differently depending on what operator you got, judge type by first char in string. If 'f' then a float */
  
  if(ptr[0]=='f'){
    printf("Float\t");
    f1=randomFloat();
    f2=randomFloat();

    /* At this point, ptr holds operator, f1 and f2 the operands. Now we work to determine the reference result. */
   
    if(strcmp(ptr,"fadd")==0){
      fresult=f1+f2;
    } else if (strcmp(ptr, "fsub")==0){
      fresult=f1-f2;
    } else if (strcmp(ptr, "fmul")==0){
      fresult=f1*f2;
    } else if (strcmp(ptr, "fdiv")==0){
      fresult=f1/f2;
    }
    printf("%s %8.8g %8.8g = %8.8g\n",ptr,f1,f2,fresult);
  } else {
    printf("Int\t");
    i1=randomInt();
    i2=randomInt();

    if(strcmp(ptr,"add")==0){
      iresult=i1+i2;
    } else if (strcmp(ptr, "sub")==0){
      iresult=i1-i2;
    } else if (strcmp(ptr, "mul")==0){
      iresult=i1*i2;
    } else if (strcmp(ptr, "div")==0){
      iresult=i1/i2;
    }

    printf("%s %d %d = %d \n",ptr,i1,i2,iresult);
  }

  /* This section shows how to read a line from stdin, process and do a similar operation as above. */
  
  char *lineBuffer=NULL;
  size_t lenBuffer=0;
  ssize_t nread=0;


  printf("Print a command: ");
  nread=getline(&lineBuffer,&lenBuffer,stdin);
  
  printf("got:> %s \n",lineBuffer);

  int rv;
  char command[10];

  
  rv=sscanf(lineBuffer,"%s",command);

  printf("Command: |%s|\n",command);
  
  if(command[0]=='f'){
    printf("Float\t");
    rv=sscanf(lineBuffer,"%s %lg %lg",command,&f1,&f2);
    if(strcmp(command,"fadd")==0){
      fresult=f1+f2;
    } else if (strcmp(command, "fsub")==0){
      fresult=f1-f2;
    } else if (strcmp(command, "fmul")==0){
      fresult=f1*f2;
    } else if (strcmp(command, "fdiv")==0){
      fresult=f1/f2;
    }
    printf("%s %8.8g %8.8g = %8.8g\n",command,f1,f2,fresult);
  } else {
    printf("Int\t");
    rv=sscanf(lineBuffer,"%s %d %d",command,&i1,&i2);
    if(strcmp(command,"add")==0){
      iresult=i1+i2;
    } else if (strcmp(command, "sub")==0){
      iresult=i1-i2;
      printf("[%s %d %d = %d ]\n",command,i1,i2,iresult);
    } else if (strcmp(command, "mul")==0){
      iresult=i1*i2;
    } else if (strcmp(command, "div")==0){
      
      iresult=i1/i2;
    } else {
      printf("No match\n");
    }

    printf("%s %d %d = %d \n",command,i1,i2,iresult);
  }
  

  
  
  free(lineBuffer); // This is needed for the getline() as it will allocate memory (if the provided buffer is NUL).


  printf("sizeof(struct calcProtocol)  = %d \n",sizeof(struct calcProtocol));
  printf("sizeof(struct calcMessage)  = %d \n",sizeof(struct calcMessage));
  

}
