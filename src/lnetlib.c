#include "luaosi/lnetlib.h"


#include "luaosi/lauxlib.h"


#define tolsock(L,i,c)	((LuaSocket *)luaL_testinstance(L, i, \
                                                        losi_SocketClasses[c]))

LOSILIB_API losi_Address *losi_newaddress (lua_State *L,
                                           losi_AddressType type)
{
	losi_Address *na;
	size_t sz;
	switch (type) {
		case LOSI_ADDRTYPE_IPV4: sz = LOSI_ADDRSIZE_IPV4; break;
		case LOSI_ADDRTYPE_IPV6: sz = LOSI_ADDRSIZE_IPV6; break;
		case LOSI_ADDRTYPE_FILE: sz = LOSI_ADDRSIZE_FILE; break;
		default: luaL_error(L, "invalid address type"); return NULL;
	}
	na = (losi_Address *)lua_newuserdata(L, sz);
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
