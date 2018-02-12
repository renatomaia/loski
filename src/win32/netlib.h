#ifndef netlib_h
#define netlib_h


#include <winsock.h>

typedef void losi_NetDriver;
typedef struct losi_Socket {
	SOCKET id;
	int blocking;
} losi_Socket;
typedef struct sockaddr losi_Address;

#include "netlibapi.h"


#endif
