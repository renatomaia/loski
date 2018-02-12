#ifndef luaosi_evtlib_h
#define luaosi_evtlib_h


#include "luaosi/evtdef.h"

#include "luaosi/errors.h"

#include <lua.h>

#ifndef LOSKI_EVTFLAGS_CUSTOM
typedef unsigned int loski_EventFlags;
#define LOSKI_EVTFLAGS_NONE	0x00
#define LOSKI_EVTFLAGS_INPUT	0x01
#define LOSKI_EVTFLAGS_OUTPUT	0x02
#endif

#ifndef LOSKI_DISABLE_EVENTDRV
LOSKIDRV_API loski_ErrorCode loskiE_initdrv (loski_EventDriver *drv);

LOSKIDRV_API void loskiE_freedrv (loski_EventDriver *drv);
#endif

LOSKIDRV_API loski_ErrorCode loskiE_initwatcher(loski_EventDriver *drv,
                                                loski_EventWatcher *watcher);

LOSKIDRV_API loski_ErrorCode loskiE_endwatcher(loski_EventDriver *drv,
                                               loski_EventWatcher *watcher);

LOSKIDRV_API loski_ErrorCode loskiE_setwatch(loski_EventDriver *drv,
                                             loski_EventWatcher *watcher,
                                             loski_EventSource *src,
                                             loski_EventFlags events);

LOSKIDRV_API loski_IntUniqueId loskiE_getsourceid(loski_EventDriver *drv,
                                                  loski_EventSource *src);

LOSKIDRV_API loski_ErrorCode loskiE_waitevent(loski_EventDriver *drv,
                                              loski_EventWatcher *watcher,
                                              lua_Number timeout);

LOSKIDRV_API void loskiE_inititerator(loski_EventDriver *drv,
                                      loski_EventWatcher *watcher,
                                      loski_EventIterator *i);

LOSKIDRV_API int loskiE_nextevent(loski_EventDriver *drv,
                                  loski_EventWatcher *watcher,
                                  loski_EventIterator *i,
                                  loski_EventSource *src,
                                  loski_EventFlags *events);


#endif
