#ifndef lnetlib_h
#define lnetlib_h


#include "loskiconf.h"

#include <netlib.h>


#define LOSKI_NETADDRCLS LOSKI_PREFIX"network.Address"


#define LOSKI_STRMSOCKET 3
#define LOSKI_BASESOCKET 4

static const char *const loski_SocketClasses[] = {
	LOSKI_PREFIX"network.ListenSocket",
	LOSKI_PREFIX"network.ConnectionSocket",
	LOSKI_PREFIX"network.DatagramSocket",
	LOSKI_PREFIX"network.StreamSocket",
	LOSKI_PREFIX"network.Socket"
};

typedef struct LuaSocket {
	loski_Socket socket;
	int closed;
} LuaSocket;


#endif
