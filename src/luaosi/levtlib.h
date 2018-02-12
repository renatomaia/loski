#ifndef luaosi_levtlib_h
#define luaosi_levtlib_h


#include "luaosi/evtlib.h"
#include "luaosi/lauxlib.h"


#define LOSI_WATCHERCLS LOSI_PREFIX"EventWatcher"

typedef struct LuaWatcher {
	losi_EventWatcher watcher;
	size_t refs;
} LuaWatcher;

LOSILIB_API losi_EventWatcher *losi_newwatcher (lua_State *L);

LOSILIB_API void losi_enablewatcher (lua_State *L);

LOSILIB_API losi_EventWatcher *losi_towatcher (lua_State *L, int idx);

#define losi_iswatcher(L,i)	(losi_towatcher(L, i) != NULL)


typedef losi_ErrorCode (*losi_GetEventSource)(void *udata, int newref, 
                                              losi_EventSource *src,
                                              losi_EventFlags evtflags);

LOSILIB_API void losi_defgetevtsrc (lua_State *L,
                                    const char *cls,
                                    losi_GetEventSource get);
LOSILIB_API losi_ErrorCode losi_getevtsrc (lua_State *L, int idx, int newref,
                                           losi_EventSource *src,
                                           losi_EventFlags evtflags);

typedef void (*losi_FreeEventSource)(void *udata);

LOSILIB_API void losi_deffreeevtsrc (lua_State *L,
                                     const char *cls,
                                     losi_FreeEventSource free);
LOSILIB_API void losi_freeevtsrc (lua_State *L, int idx);


#endif
