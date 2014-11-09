#ifndef eventlibapi_h
#define eventlibapi_h

#include <stdio.h> /* definition of 'FILE*' */
#include <lua.h> /* to copy error messages to Lua */

#include "timelib.h"
#include "proclib.h"
#include "netlib.h"

enum loski_WatchableKind {
	LOSKI_WATCHFILE,
	LOSKI_WATCHSOCKET,
	LOSKI_WATCHPROCESS
};
typedef enum loski_WatchableKind loski_WatchableKind;

union loski_WatchableReference {
	FILE *file;
	loski_Socket *socket;
	loski_Process *process;
};
typedef union loski_WatchableReference loski_WatchableReference;

union loski_WatchableObject {
	loski_WatchableFile file;
	loski_WatchableSocket socket;
	loski_WatchableProcess process;
};
typedef union loski_WatchableObject loski_WatchableObject;

typedef int loski_WatchableEvent;

struct loski_EventWatch {
	loski_WatchableKind kind;
	loski_WatchableObject object;
	loski_WatchableEvent event;
};
typedef struct loski_EventWatch loski_EventWatch;


LOSKIDRV_API int loski_openevents(loski_EventDriver *drv);

LOSKIDRV_API int loski_closeevents(loski_EventDriver *drv);

LOSKIDRV_API int loski_eventerror(int error, lua_State *L);

LOSKIDRV_API int loski_initwatcher(loski_EventDriver *drv,
                                   loski_EventWatcher *watcher);

LOSKIDRV_API int loski_endwatcher(loski_EventDriver *drv,
                                  loski_EventWatcher *watcher);

LOSKIDRV_API void loski_pushwachedkey(lua_State *L,
                                      loski_EventWatch *watch);

LOSKIDRV_API int loski_addwatch(loski_EventDriver *drv,
                                loski_EventWatcher *watcher,
                                loski_EventWatch *watch,
                                loski_WatchableReference *ref);

LOSKIDRV_API int loski_delwatch(loski_EventDriver *drv,
                                loski_EventWatcher *watcher,
                                loski_EventWatch *watch,
                                loski_WatchableReference *ref);

LOSKIDRV_API size_t loski_eventqueuesize(loski_EventDriver *drv,
                                         loski_EventWatcher *watcher,
                                         size_t count);

LOSKIDRV_API int loski_waitevent(loski_EventDriver *drv,
                                 loski_EventWatcher *watcher,
                                 void *queue,
                                 size_t *count,
                                 lua_Number timeout);

LOSKIDRV_API int loski_getevent(loski_EventDriver *drv,
                                loski_EventWatch *watch,
                                void *queue,
                                size_t index);

#endif
