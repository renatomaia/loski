#ifndef luaosi_evtlib_h
#define luaosi_evtlib_h


#include "luaosi/evtdef.h"

#include "luaosi/errors.h"

#include <lua.h>

#ifndef LOSI_EVTFLAGS_CUSTOM
typedef unsigned int losi_EventFlags;
#define LOSI_EVTFLAGS_NONE	0x00
#define LOSI_EVTFLAGS_INPUT	0x01
#define LOSI_EVTFLAGS_OUTPUT	0x02
#endif

#ifndef LOSI_DISABLE_EVENTDRV
LOSIDRV_API losi_ErrorCode losiE_initdrv (losi_EventDriver *drv);

LOSIDRV_API void losiE_freedrv (losi_EventDriver *drv);
#endif

LOSIDRV_API losi_ErrorCode losiE_initwatcher(losi_EventDriver *drv,
                                             losi_EventWatcher *watcher);

LOSIDRV_API losi_ErrorCode losiE_endwatcher(losi_EventDriver *drv,
                                            losi_EventWatcher *watcher);

LOSIDRV_API losi_ErrorCode losiE_setwatch(losi_EventDriver *drv,
                                          losi_EventWatcher *watcher,
                                          losi_EventSource *src,
                                          losi_EventFlags events);

LOSIDRV_API losi_IntUniqueId losiE_getsourceid(losi_EventDriver *drv,
                                               losi_EventSource *src);

LOSIDRV_API losi_ErrorCode losiE_waitevent(losi_EventDriver *drv,
                                           losi_EventWatcher *watcher,
                                           lua_Number timeout);

LOSIDRV_API void losiE_inititerator(losi_EventDriver *drv,
                                    losi_EventWatcher *watcher,
                                    losi_EventIterator *i);

LOSIDRV_API int losiE_nextevent(losi_EventDriver *drv,
                                losi_EventWatcher *watcher,
                                losi_EventIterator *i,
                                losi_EventSource *src,
                                losi_EventFlags *events);


#endif
