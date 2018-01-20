#include "lnetaux.h"


#include "loskiaux.h"


#define tolsock(L,i,c)	((LuaSocket *)luaL_testinstance(L, i, \
                                                        loski_SocketClasses[c]))

LOSKILIB_API loski_Address *loski_newaddress (lua_State *L)
{
	loski_Address *na = (loski_Address *)lua_newuserdata(L,sizeof(loski_Address));
	luaL_setmetatable(L, LOSKI_NETADDRCLS);
	return na;
}


LOSKILIB_API loski_Socket *loski_newsocket (lua_State *L, int cls)
{
	LuaSocket *ls = (LuaSocket *)lua_newuserdata(L, sizeof(LuaSocket));
	ls->closed = 1;
	luaL_setmetatable(L, loski_SocketClasses[cls]);
	return &ls->socket;
}

LOSKILIB_API void loski_enablesocket (lua_State *L, int idx)
{
	LuaSocket *ls = tolsock(L, idx, LOSKI_SOCKTYPE_SOCK);
	if (ls) ls->closed = 0;
}

LOSKILIB_API loski_Socket *loski_tosocket (lua_State *L, int idx, int cls)
{
	LuaSocket *ls = tolsock(L, idx, cls);
	if (!ls || ls->closed) return NULL;
	return &ls->socket;
}
