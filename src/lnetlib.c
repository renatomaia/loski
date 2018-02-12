#include "luaosi/lnetlib.h"


#include "luaosi/lauxlib.h"


#define tolsock(L,i,c)	((LuaSocket *)luaL_testinstance(L, i, \
                                                        losi_SocketClasses[c]))

LOSILIB_API losi_Address *losi_newaddress (lua_State *L)
{
	losi_Address *na = (losi_Address *)lua_newuserdata(L,sizeof(losi_Address));
	luaL_setmetatable(L, LOSI_NETADDRCLS);
	return na;
}


LOSILIB_API losi_Socket *losi_newsocket (lua_State *L, int cls)
{
	LuaSocket *ls = (LuaSocket *)lua_newuserdata(L, sizeof(LuaSocket));
	ls->refs = 0;
	luaL_setmetatable(L, losi_SocketClasses[cls]);
	return &ls->socket;
}

LOSILIB_API void losi_enablesocket (lua_State *L, int idx)
{
	LuaSocket *ls = tolsock(L, idx, LOSI_SOCKTYPE_SOCK);
	if (ls) ++(ls->refs);
}

LOSILIB_API losi_Socket *losi_tosocket (lua_State *L, int idx, int cls)
{
	LuaSocket *ls = tolsock(L, idx, cls);
	if (!ls || ls->refs == 0) return NULL;
	return &ls->socket;
}
