#ifndef eventlib_h
#define eventlib_h


#include "eventdef.h"

#include "timeaux.h"
#include "loskierr.h"

#ifndef LOSKI_EVTVAL_CUSTOM
typedef enum loski_EventValue {
	LOSKI_EVTVAL_INPUT,
	LOSKI_EVTVAL_OUTPUT
} loski_EventValue;
#endif

#ifndef LOSKI_DISABLE_EVENTDRV
LOSKIDRV_API loski_ErrorCode loskiE_initdrv (loski_EventDriver *drv);

LOSKIDRV_API void loskiE_freedrv (loski_EventDriver *drv);
#endif

LOSKIDRV_API loski_ErrorCode loskiE_initwatcher(loski_EventDriver *drv,
                                                loski_EventWatcher *watcher);

LOSKIDRV_API loski_ErrorCode loskiE_endwatcher(loski_EventDriver *drv,
                                               loski_EventWatcher *watcher);

LOSKIDRV_API loski_ErrorCode loskiE_addwatch(loski_EventDriver *drv,
                                             loski_EventWatcher *watcher,
                                             loski_EventSource *source,
                                             loski_EventValue event);

LOSKIDRV_API loski_ErrorCode loskiE_delwatch(loski_EventDriver *drv,
                                             loski_EventWatcher *watcher,
                                             loski_EventSource *source,
                                             loski_EventValue event);

LOSKIDRV_API loski_IntUniqueId loskiE_getsourceid(loski_EventDriver *drv,
                                                  loski_EventSource *source);

LOSKIDRV_API loski_ErrorCode loskiE_waitevent(loski_EventDriver *drv,
                                              loski_EventWatcher *watcher,
                                              lua_Number timeout);

LOSKIDRV_API int loskiE_nextevent(loski_EventDriver *drv,
                                  loski_EventWatcher *watcher,
                                  size_t *index,
                                  loski_EventSource *source,
                                  loski_EventValue *event);


#endif
