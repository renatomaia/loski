#include "luaosi/levtlib.h"


#define tolwatcher(L,i)	((LuaWatcher *)luaL_testudata(L, i, LOSI_WATCHERCLS))

LOSILIB_API losi_EventWatcher *losi_newwatcher (lua_State *L)
{
	LuaWatcher *lw = (LuaWatcher *)lua_newuserdata(L, sizeof(LuaWatcher));
	lw->refs = 0;
	luaL_setmetatable(L, LOSI_WATCHERCLS);
	return &lw->watcher;
}

LOSILIB_API void losi_enablewatcher (lua_State *L)
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

LOSILIB_API losi_EventWatcher *losi_towatcher (lua_State *L, int idx)
{
	LuaWatcher *lw = tolwatcher(L, idx);
	if (!lw || lw->refs == 0) return NULL;
	return &lw->watcher;
}

LOSILIB_API void losi_defgetevtsrc (lua_State *L,
                                    const char *cls,
                                    losi_GetEventSource get)
{
	luaL_setclassdata(L, cls, "getevtsrc", get);
}

LOSILIB_API losi_ErrorCode losi_getevtsrc (lua_State *L, int idx, int newref,
                                           losi_EventSource *src,
                                           losi_EventFlags evtflags)
{
	void *udata;
	losi_GetEventSource get = (losi_GetEventSource)
	                           luaL_getclassdata(L, idx, "getevtsrc", &udata);
	if (get) return get(udata, newref, src, evtflags);
	return LOSI_ERRINVALID;
}

LOSILIB_API void losi_deffreeevtsrc (lua_State *L,
                                     const char *cls,
                                     losi_FreeEventSource free)
{
	luaL_setclassdata(L, cls, "freeevtsrc", free);
}

LOSILIB_API void losi_freeevtsrc (lua_State *L, int idx)
{
	void *udata;
	losi_FreeEventSource free = (losi_FreeEventSource)
	                             luaL_getclassdata(L, idx, "freeevtsrc", &udata);
	if (free) free(udata);
}
