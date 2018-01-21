#include "luaosi/levtlib.h"


#define tolwatcher(L,i)	((LuaWatcher *)luaL_testudata(L, i, LOSKI_WATCHERCLS))

LOSKILIB_API loski_EventWatcher *loski_newwatcher (lua_State *L)
{
	LuaWatcher *lw = (LuaWatcher *)lua_newuserdata(L, sizeof(LuaWatcher));
	lw->closed = 1;
	luaL_setmetatable(L, LOSKI_WATCHERCLS);
	return &lw->watcher;
}

LOSKILIB_API void loski_enablewatcher (lua_State *L, int idx)
{
	LuaWatcher *lw = tolwatcher(L, idx);
	if (lw) {
		lw->closed = 0;
		lua_newtable(L);
		lua_setuservalue(L, idx);
	}
}

LOSKILIB_API loski_EventWatcher *loski_towatcher (lua_State *L, int idx)
{
	LuaWatcher *lw = tolwatcher(L, idx);
	if (!lw || lw->closed) return NULL;
	return &lw->watcher;
}
