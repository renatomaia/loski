#ifndef netlib_h
#define netlib_h


#include <sys/socket.h>
#include <netinet/in.h>

typedef void loski_NetState;
typedef struct sockaddr loski_Address;
typedef in_port_t loski_AddressPort;
typedef int loski_Socket;

#include "netlibapi.h"


#endif
