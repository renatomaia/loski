#ifndef lnetaux_h
#define lnetaux_h


#include "loskiconf.h"

#include <netlib.h>
#include <lua.h>


#define LOSKI_NETADDRCLS LOSKI_PREFIX"NetworkAddress"

LOSKILIB_API int loski_isaddress (lua_State *L, int idx);
LOSKILIB_API loski_Address *loski_toaddress (lua_State *L, int idx);


/* superclasses used only in Lua */
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

LOSKILIB_API int loski_issocket (lua_State *L, int idx, int cls);
LOSKILIB_API loski_Socket *loski_tosocket (lua_State *L, int idx, int cls);


#endif
