#include "luaosi/evtlib.h"


#include "luaosi/fdlib.h"
#include "luaosi/timelib.h"
#include "luaosi/ltimelib.h"
#include "luaosi/levtlib.h"

#include <assert.h>
#include <errno.h>
#include <sys/select.h>

LOSIDRV_API losi_ErrorCode losiE_initdrv (losi_EventDriver *drv)
{
	/* nothing to do */
	return LOSI_ERRNONE;
}

LOSIDRV_API void losiE_freedrv (losi_EventDriver *drv)
{
	/* nothing to do */
}

LOSIDRV_API losi_ErrorCode losiE_initwatcher(losi_EventDriver *drv,
                                             losi_EventWatcher *watcher)
{
	watcher->maxfd = 0;
	FD_ZERO(&watcher->sets[0]);
	FD_ZERO(&watcher->sets[1]);
	watcher->maxvisited = 0;
	watcher->unvisited = 0;
	FD_ZERO(&watcher->selected[0]);
	FD_ZERO(&watcher->selected[1]);
	return LOSI_ERRNONE;
}

LOSIDRV_API losi_ErrorCode losiE_endwatcher(losi_EventDriver *drv,
                                            losi_EventWatcher *watcher)
{
	return LOSI_ERRNONE;
}

LOSIDRV_API losi_ErrorCode losiE_setwatch(losi_EventDriver *drv,
                                          losi_EventWatcher *watcher,
                                          losi_EventSource *src,
                                          losi_EventFlags evtflags)
{
	int fd = *src;
	if (fd < 0 || fd >= FD_SETSIZE) return LOSI_ERRINVALID;
	if (evtflags & LOSI_EVTFLAGS_INPUT) FD_SET(fd, &watcher->sets[0]);
	else                                FD_CLR(fd, &watcher->sets[0]);
	if (evtflags & LOSI_EVTFLAGS_OUTPUT) FD_SET(fd, &watcher->sets[1]);
	else                                 FD_CLR(fd, &watcher->sets[1]);
	if (evtflags != LOSI_EVTFLAGS_NONE) {
		if (fd >= watcher->maxfd) watcher->maxfd = fd+1;
	} else if (fd+1 == watcher->maxfd) {
		int i;
		for (i=fd-1; i>=0; --i)
			if (FD_ISSET(i, &watcher->sets[0]) || FD_ISSET(i, &watcher->sets[1]))
				break;
		watcher->maxfd = i+1;
	}
	return LOSI_ERRNONE;
}

LOSIDRV_API losi_IntUniqueId losiE_getsourceid(losi_EventDriver *drv,
                                               losi_EventSource *src)
{
	return (losi_IntUniqueId)*src;
}

LOSIDRV_API losi_ErrorCode losiE_waitevent(losi_EventDriver *drv,
                                           losi_EventWatcher *watcher,
                                           lua_Number timeout)
{
	if (watcher->maxfd == 0) return LOSI_ERRINVALID;
	watcher->maxvisited = 0;
	watcher->selected[0] = watcher->sets[0];
	watcher->selected[1] = watcher->sets[1];
again:
	if (timeout >= 0) {
		lua_Number start = losiT_now(NULL);
		struct timeval tm;
		losi_seconds2timeval(timeout, &tm);
		watcher->unvisited = select(watcher->maxfd,
		                            &watcher->selected[0],
		                            &watcher->selected[1],
		                            NULL, &tm);
		timeout -= (losiT_now(NULL) - start);
		if (timeout < 0) timeout = 0;
	} else {
		watcher->unvisited = select(watcher->maxfd,
		                            &watcher->selected[0],
		                            &watcher->selected[1],
		                            NULL, NULL);
	}
	if (watcher->unvisited != -1) return LOSI_ERRNONE;
	switch (errno) {
		case EBADF:
		case EINVAL: return LOSI_ERRINVALID;
		case EINTR: if (timeout == 0) return LOSI_ERRUNFULFILLED;
		            else goto again;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API void losiE_inititerator(losi_EventDriver *drv,
                                    losi_EventWatcher *watcher,
                                    losi_EventIterator *i)
{
	*i = 0;
}

LOSIDRV_API int losiE_nextevent(losi_EventDriver *drv,
                                losi_EventWatcher *watcher,
                                losi_EventIterator *i,
                                losi_EventSource *src,
                                losi_EventFlags *evtflags)
{
	while (1) {
		int fd = (*i)++;
		if (fd >= watcher->maxvisited && watcher->unvisited <= 0) break;
		*evtflags = LOSI_EVTFLAGS_NONE;
		if (FD_ISSET(fd, &watcher->selected[0])) {
			*evtflags |= LOSI_EVTFLAGS_INPUT;
			--watcher->unvisited;
		}
		if (FD_ISSET(fd, &watcher->selected[1])) {
			*evtflags |= LOSI_EVTFLAGS_OUTPUT;
			--watcher->unvisited;
		}
		if (*evtflags != LOSI_EVTFLAGS_NONE) {
			if (fd >= watcher->maxvisited) watcher->maxvisited = fd + 1;
			*src = fd;
			return 1;
		}
	}
	return 0;
}
