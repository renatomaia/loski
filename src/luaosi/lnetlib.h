#ifndef luaosi_lnetlib_h
#define luaosi_lnetlib_h


#include "luaosi/config.h"
#include "luaosi/netlib.h"

#include <lua.h>
#include <lauxlib.h>


#define LOSKI_NETADDRCLS LOSKI_PREFIX"NetworkAddress"

#define loski_toaddress(L,i)	((loski_Address *) \
                               luaL_testudata(L, i, LOSKI_NETADDRCLS))

#define loski_isaddress(L,i)	(loski_toaddress(L, i) != NULL)

LOSKILIB_API loski_Address *loski_newaddress (lua_State *L);


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

LOSKILIB_API loski_Socket *loski_newsocket (lua_State *L, int cls);

LOSKILIB_API void loski_enablesocket (lua_State *L, int idx);

LOSKILIB_API loski_Socket *loski_tosocket (lua_State *L, int idx, int cls);

#define loski_issocket(L,i,c)	(loski_tosocket(L, i, c) != NULL)


#endif
