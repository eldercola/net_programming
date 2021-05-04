#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Here we use " as the calcLib.c and calcLib.h files are in the same folder, and are to be BUILT
   to into a library, that will be included in other files. 

   This is a C lib, and will be built as such.
   
*/ 
#include "calcLib.h"


/* array of char* that points to char arrays.  */ 
char *arith[]={"add","div","mul","sub","fadd","fdiv","fmul","fsub"};

/* Used for random number */
time_t myData_seedValue;

int initCalcLib(void){
  /* Init the random number generator with a seed, based on the current time--> should be randomish each time called */
  srand((unsigned) time(&myData_seedValue));
  return(0);
}

int initCalcLib_seed(unsigned int seed){
  /* 
     Init the random number generator with a FIXED seed, will allow us to grab random numbers 
     in the same sequence all the time. Good when debugging, bad when running live. 

     This is 'messy' for more details see https://en.wikipedia.org/wiki/Pseudorandom_number_generator. 

     DO NOT USE rand() for production, wher you NEED good random numbers. 
  */
  
  myData_seedValue=seed;
  srand(seed);
  return(0);
}
  
char *randomType(void){
  int Listitems=sizeof(arith)/(sizeof(char*)); 
  /* Figure out HOW many entries there are in the list.
     First we get the total size that the array of pointers use, sizeof(arith). Then we divide with 
     the size of a pointer (sizeof(char*)), this gives us the number of pointers in the list. 
  */
  int itemPos=rand() % Listitems;
  /* As we know the number of items, we can just draw a random number and modulo it with the number 
     of items in the list, then we will get a random number between 0 and the number of items in the list 
     
     Using that information, we just return the string found at that position arith[itemPos];
  */
  return(arith[itemPos]);
  
};


int randomInt(void){
  /* Draw a random interger between o and RAND_MAX, then modulo this with 100 to get a random 
     number between 0 and 100. */
  
  return( rand()%100 );
};


double randomFloat(void){
  /* The same as for the interber, but for a double, and without the modulo. We cant use 
     the module approach as it would generate integers, which we do not want. */
  double x = (double)rand()/(double)(RAND_MAX/100.0);
  return(x);
};


