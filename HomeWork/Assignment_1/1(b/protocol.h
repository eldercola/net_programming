
#ifdef __GCC_IEC_559 
#pragma message("GCC ICE 559 defined...")

#else

#error *** do not use this platform

#endif


#include <stdint.h>

/* 
   Used in both directions; if 
   server->client,type should be set to 1, 
   client->server type = 2. 
 */
struct  __attribute__((__packed__)) calcProtocol{
  uint16_t type;  // What message is this, 1 = server to client, 2 client to server, 3... reserved , conversion needed (for practice)
  uint16_t major_version; // 1, conversion needed (for practice)
  uint16_t minor_version; // 0, conversion needed (for practice)
  uint32_t id; // Server side identification with operation. Client must return the same ID as it got from Server., conversion needed (for practice)
  uint32_t arith; // What operation to perform, see mapping below. 
  int32_t inValue1; // integer value 1, conversion needed (for practice)
  int32_t inValue2; // integer value 2, conversion needed (for practice)
  int32_t inResult; // integer result, conversion needed (for practice)
  double flValue1;  // float value 1,NO NEED TO do host to Network or Network to Host conversion here, we are using equivalent platforms        
  double flValue2;  // float value 2,NO NEED TO do host to Network or Network to Host conversion here, we are using equivalent platforms
  double flResult;  // float result,NO NEED TO do host to Network or Network to Host conversion here, we are using equivalent platforms
};

  
struct  __attribute__((__packed__)) calcMessage {
  uint16_t type;    // See below, conversion needed (for practice)
  uint32_t message; // See below, conversion needed (for practice)
  
  // Protocol, UDP = 17, TCP = 6, other values are reserved. 
  uint16_t protocol; // conversion needed (for practice)
  uint16_t major_version; // 1, conversion needed (for practice)
  uint16_t minor_version; // 0 , conversion needed (for practice)

};


/* arith mapping in calcProtocol
1 - add
2 - sub
3 - mul
4 - div
5 - fadd
6 - fsub
7 - fmul
8 - fdiv

other numbers are reserved

*/


/* 
   calcMessage.type
   1 - server-to-client, text protocol
   2 - server-to-client, binary protocol
   3 - server-to-client, N/A
   21 - client-to-server, text protocol
   22 - client-to-server, binary protocol
   23 - client-to-serve, N/A
   
   calcMessage.message 

   0 = Not applicable/availible (N/A or NA)
   1 = OK   // Accept 
   2 = NOT OK  // Reject 

*/
