#ifndef netlib_h
#define netlib_h


#include <winsock.h>

typedef void loski_NetDriver;
typedef struct loski_Socket {
	SOCKET id;
	int blocking;
} loski_Socket;
typedef struct sockaddr loski_Address;

#include "netlibapi.h"


#endif
