#include "luaosi/levtlib.h"


#define tolwatcher(L,i)	((LuaWatcher *)luaL_testudata(L, i, LOSKI_WATCHERCLS))

LOSKILIB_API loski_EventWatcher *loski_newwatcher (lua_State *L)
{
	LuaWatcher *lw = (LuaWatcher *)lua_newuserdata(L, sizeof(LuaWatcher));
	lw->refs = 0;
	luaL_setmetatable(L, LOSKI_WATCHERCLS);
	return &lw->watcher;
}

LOSKILIB_API void loski_enablewatcher (lua_State *L)
{
	LuaWatcher *lw = tolwatcher(L, -1);
	if (lw) {
		++(lw->refs);
		lua_newtable(L);      /* t = {} */
		lua_pushvalue(L, -1);
		lua_newtable(L);
		lua_settable(L, -3);  /* t[t] = {} */
		lua_setuservalue(L, -2);
	}
}

LOSKILIB_API loski_EventWatcher *loski_towatcher (lua_State *L, int idx)
{
	LuaWatcher *lw = tolwatcher(L, idx);
	if (!lw || lw->refs == 0) return NULL;
	return &lw->watcher;
}

LOSKILIB_API void loski_defgetevtsrc (lua_State *L,
                                      const char *cls,
                                      loski_GetEventSource get)
{
	luaL_setclassdata(L, cls, "getevtsrc", get);
}

LOSKILIB_API loski_ErrorCode loski_getevtsrc (lua_State *L, int idx, int newref,
                                              loski_EventSource *src,
                                              loski_EventFlags evtflags)
{
	void *udata;
	loski_GetEventSource get = (loski_GetEventSource)
	                           luaL_getclassdata(L, idx, "getevtsrc", &udata);
	if (get) return get(udata, newref, src, evtflags);
	return LOSKI_ERRINVALID;
}

LOSKILIB_API void loski_deffreeevtsrc (lua_State *L,
                                       const char *cls,
                                       loski_FreeEventSource free)
{
	luaL_setclassdata(L, cls, "freeevtsrc", free);
}

LOSKILIB_API void loski_freeevtsrc (lua_State *L, int idx)
{
	void *udata;
	loski_FreeEventSource free = (loski_FreeEventSource)
	                             luaL_getclassdata(L, idx, "freeevtsrc", &udata);
	if (free) free(udata);
}
