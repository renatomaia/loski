#ifndef luaosi_levtlib_h
#define luaosi_levtlib_h


#include "luaosi/evtlib.h"
#include "luaosi/lauxlib.h"


#define LOSKI_WATCHERCLS LOSKI_PREFIX"EventWatcher"

typedef struct LuaWatcher {
	loski_EventWatcher watcher;
	int closed;
} LuaWatcher;

LOSKILIB_API loski_EventWatcher *loski_newwatcher (lua_State *L);

LOSKILIB_API void loski_enablewatcher (lua_State *L);

LOSKILIB_API loski_EventWatcher *loski_towatcher (lua_State *L, int idx);

#define loski_iswatcher(L,i)	(loski_towatcher(L, i) != NULL)


#define LOSKI_EVENTSOURCECONV LOSKI_PREFIX"ToEventSourceOperation"

typedef int (*loski_EventSourceConv)(lua_State *L, int idx, 
                                     loski_EventSource *source);

#define loski_seteventsourceconv(L, cls, func) \
	loskiL_setclassop(L, LOSKI_EVENTSOURCECONV, cls, func)

#define loski_geteventsourceconv(L) \
	loskiL_getvalueop(L, LOSKI_EVENTSOURCECONV)


#endif
