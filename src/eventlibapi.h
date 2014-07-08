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

union loski_WatchableObject {
	FILE *file;
	loski_Socket *socket;
	loski_Process *process;
};
typedef union loski_WatchableObject loski_WatchableObject;

typedef int loski_WatchableEvent;

struct loski_EventWatch {
	loski_WatchableKind kind;
	loski_WatchableObject object;
	loski_WatchableEvent event;
};
typedef struct loski_EventWatch loski_EventWatch;


LOSKIDRV_API int loski_openevents();

LOSKIDRV_API int loski_closeevents();

LOSKIDRV_API int loski_eventerror(int error, lua_State *L);

LOSKIDRV_API int loski_initwatcher(loski_EventWatcher *watcher);

LOSKIDRV_API int loski_endwatcher(loski_EventWatcher *watcher);

LOSKIDRV_API int loski_addwatch(loski_EventWatcher *watcher,
                              loski_EventWatch *watch);

LOSKIDRV_API int loski_delwatch(loski_EventWatcher *watcher,
                              loski_EventWatch *watch);

LOSKIDRV_API size_t loski_eventqueuesize(loski_EventWatcher *watcher,
                                       size_t count);

LOSKIDRV_API int loski_waitevent(loski_EventWatcher *watcher,
                               void *queue,
                               size_t *count,
                               loski_Seconds timeout);

LOSKIDRV_API int loski_getevent(void *queue,
                              size_t index,
                              loski_EventWatch *watch);

#endif
