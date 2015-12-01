#ifndef lnetlib_h
#define lnetlib_h


#include "loskiconf.h"

#include <netlib.h>


#define LOSKI_NETADDRCLS LOSKI_PREFIX"NetworkAddress"


#define LOSKI_SOCKTYPE_STRM 3
#define LOSKI_SOCKTYPE_SOCK 4

static const char *const loski_SocketClasses[] = {
	LOSKI_PREFIX"ListenSocket",
	LOSKI_PREFIX"ConnectionSocket",
	LOSKI_PREFIX"DatagramSocket",
	LOSKI_PREFIX"StreamSocket",
	LOSKI_PREFIX"NetworkSocket"
};

typedef struct LuaSocket {
	loski_Socket socket;
	int closed;
} LuaSocket;


#endif
