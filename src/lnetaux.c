#include "lnetaux.h"
#include "lnetlib.h"

#include <lauxlib.h>


LOSKILIB_API int loski_isaddress (lua_State *L, int idx)
{
	return luaL_testudata(L, idx, LOSKI_NETADDRCLS) != NULL;
}

LOSKILIB_API loski_Address *loski_toaddress (lua_State *L, int idx)
{
	if (!loski_isaddress(L, idx)) return NULL;
	return (loski_Address *)lua_touserdata(L, idx);
}


LOSKILIB_API int loski_issocket (lua_State *L, int idx, int cls)
{
	LuaSocket *ls = ((LuaSocket *)luaL_testinstance(L, idx,
	                                                loski_SocketClasses[cls]));
	return (ls != NULL) && (!ls->closed);
}

LOSKILIB_API loski_Socket *loski_tosocket (lua_State *L, int idx, int cls)
{
	LuaSocket *ls;
	if (!loski_issocket(L, idx, cls)) return NULL;
	ls = ((LuaSocket *)lua_touserdata(L, idx));
	return &ls->socket;
}
