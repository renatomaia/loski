#ifndef netlib_h
#define netlib_h


#include <winsock.h>

#define LOSKI_SOCKETSTRING "%d"

struct loski_Socket {
	SOCKET id;
	int blocking;
};

typedef struct loski_Socket loski_Socket;
typedef struct sockaddr loski_Address;

#include "netlibapi.h"

#define loski_pushsocketkey(L,ps) lua_pushinteger(L,*ps)


#endif
