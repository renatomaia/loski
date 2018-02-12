#ifndef luaosi_lnetlib_h
#define luaosi_lnetlib_h


#include "luaosi/config.h"
#include "luaosi/netlib.h"

#include <lua.h>
#include <lauxlib.h>


#define LOSI_NETADDRCLS LOSI_PREFIX"NetworkAddress"

#define losi_toaddress(L,i)	((losi_Address *) \
                            luaL_testudata(L, i, LOSI_NETADDRCLS))

#define losi_isaddress(L,i)	(losi_toaddress(L, i) != NULL)

LOSILIB_API losi_Address *losi_newaddress (lua_State *L);


/* superclasses used only in Lua */
#define LOSI_SOCKTYPE_STRM 3
#define LOSI_SOCKTYPE_SOCK 4

static const char *const losi_SocketClasses[] = {
	LOSI_PREFIX"ListenSocket",
	LOSI_PREFIX"ConnectionSocket",
	LOSI_PREFIX"DatagramSocket",
	LOSI_PREFIX"StreamSocket",
	LOSI_PREFIX"NetworkSocket"
};

typedef struct LuaSocket {
	losi_Socket socket;
	size_t refs;
} LuaSocket;

LOSILIB_API losi_Socket *losi_newsocket (lua_State *L, int cls);

LOSILIB_API void losi_enablesocket (lua_State *L, int idx);

LOSILIB_API losi_Socket *losi_tosocket (lua_State *L, int idx, int cls);

#define losi_issocket(L,i,c)	(losi_tosocket(L, i, c) != NULL)


#endif
