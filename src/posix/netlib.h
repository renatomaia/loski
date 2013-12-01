#ifndef netlib_h
#define netlib_h


#include <sys/socket.h>

#define LOSKI_SOCKETSTRING "%d"

typedef int loski_Socket;
typedef struct sockaddr loski_Address;

#include "netlibapi.h"

#define loski_pushsocketkey(L,ps) lua_pushinteger(L,*ps)


#endif
