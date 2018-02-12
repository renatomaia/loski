#ifndef luaosi_levtlib_h
#define luaosi_levtlib_h


#include "luaosi/evtlib.h"
#include "luaosi/lauxlib.h"


#define LOSKI_WATCHERCLS LOSKI_PREFIX"EventWatcher"

typedef struct LuaWatcher {
	loski_EventWatcher watcher;
	size_t refs;
} LuaWatcher;

LOSKILIB_API loski_EventWatcher *loski_newwatcher (lua_State *L);

LOSKILIB_API void loski_enablewatcher (lua_State *L);

LOSKILIB_API loski_EventWatcher *loski_towatcher (lua_State *L, int idx);

#define loski_iswatcher(L,i)	(loski_towatcher(L, i) != NULL)


typedef loski_ErrorCode (*loski_GetEventSource)(void *udata, int newref, 
                                                loski_EventSource *src,
                                                loski_EventFlags evtflags);

LOSKILIB_API void loski_defgetevtsrc (lua_State *L,
                                      const char *cls,
                                      loski_GetEventSource get);
LOSKILIB_API loski_ErrorCode loski_getevtsrc (lua_State *L, int idx, int newref,
                                              loski_EventSource *src,
                                              loski_EventFlags evtflags);

typedef void (*loski_FreeEventSource)(void *udata);

LOSKILIB_API void loski_deffreeevtsrc (lua_State *L,
                                       const char *cls,
                                       loski_FreeEventSource free);
LOSKILIB_API void loski_freeevtsrc (lua_State *L, int idx);


#endif
