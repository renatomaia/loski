#include "luaosi/evtlib.h"


#include "luaosi/fdlib.h"
#include "luaosi/timelib.h"
#include "luaosi/ltimelib.h"
#include "luaosi/levtlib.h"

#include <assert.h>
#include <errno.h>
#include <sys/select.h>

LOSKIDRV_API loski_ErrorCode loskiE_initdrv (loski_EventDriver *drv)
{
	/* nothing to do */
	return LOSKI_ERRNONE;
}

LOSKIDRV_API void loskiE_freedrv (loski_EventDriver *drv)
{
	/* nothing to do */
}

LOSKIDRV_API loski_ErrorCode loskiE_initwatcher(loski_EventDriver *drv,
                                                loski_EventWatcher *watcher)
{
	watcher->maxfd = 0;
	FD_ZERO(&watcher->sets[0]);
	FD_ZERO(&watcher->sets[1]);
	watcher->maxvisited = 0;
	watcher->unvisited = 0;
	FD_ZERO(&watcher->selected[0]);
	FD_ZERO(&watcher->selected[1]);
	return LOSKI_ERRNONE;
}

LOSKIDRV_API loski_ErrorCode loskiE_endwatcher(loski_EventDriver *drv,
                                               loski_EventWatcher *watcher)
{
	return LOSKI_ERRNONE;
}

LOSKIDRV_API loski_ErrorCode loskiE_setwatch(loski_EventDriver *drv,
                                             loski_EventWatcher *watcher,
                                             loski_EventSource *src,
                                             loski_EventFlags evtflags)
{
	int fd = *src;
	if (fd < 0 || fd >= FD_SETSIZE) return LOSKI_ERRINVALID;
	if (evtflags & LOSKI_EVTFLAGS_INPUT) FD_SET(fd, &watcher->sets[0]);
	else                                 FD_CLR(fd, &watcher->sets[0]);
	if (evtflags & LOSKI_EVTFLAGS_OUTPUT) FD_SET(fd, &watcher->sets[1]);
	else                                  FD_CLR(fd, &watcher->sets[1]);
	if (evtflags != LOSKI_EVTFLAGS_NONE) {
		if (fd >= watcher->maxfd) watcher->maxfd = fd+1;
	} else if (fd+1 == watcher->maxfd) {
		int i;
		for (i=fd-1; i>=0; --i)
			if (FD_ISSET(i, &watcher->sets[0]) || FD_ISSET(i, &watcher->sets[1]))
				break;
		watcher->maxfd = i+1;
	}
	return LOSKI_ERRNONE;
}

LOSKIDRV_API loski_IntUniqueId loskiE_getsourceid(loski_EventDriver *drv,
                                                  loski_EventSource *src)
{
	return (loski_IntUniqueId)*src;
}

LOSKIDRV_API loski_ErrorCode loskiE_waitevent(loski_EventDriver *drv,
                                              loski_EventWatcher *watcher,
                                              lua_Number timeout)
{
	if (watcher->maxfd == 0) return LOSKI_ERRINVALID;
	watcher->maxvisited = 0;
	watcher->selected[0] = watcher->sets[0];
	watcher->selected[1] = watcher->sets[1];
again:
	if (timeout >= 0) {
		lua_Number start = loskiT_now(NULL);
		struct timeval tm;
		loski_seconds2timeval(timeout, &tm);
		watcher->unvisited = select(watcher->maxfd,
		                            &watcher->selected[0],
		                            &watcher->selected[1],
		                            NULL, &tm);
		timeout -= (loskiT_now(NULL) - start);
		if (timeout < 0) timeout = 0;
	} else {
		watcher->unvisited = select(watcher->maxfd,
		                            &watcher->selected[0],
		                            &watcher->selected[1],
		                            NULL, NULL);
	}
	if (watcher->unvisited != -1) return LOSKI_ERRNONE;
	switch (errno) {
		case EBADF:
		case EINVAL: return LOSKI_ERRINVALID;
		case EINTR: if (timeout == 0) return LOSKI_ERRUNFULFILLED;
		            else goto again;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API void loskiE_inititerator(loski_EventDriver *drv,
                                      loski_EventWatcher *watcher,
                                      loski_EventIterator *i)
{
	*i = 0;
}

LOSKIDRV_API int loskiE_nextevent(loski_EventDriver *drv,
                                  loski_EventWatcher *watcher,
                                  loski_EventIterator *i,
                                  loski_EventSource *src,
                                  loski_EventFlags *evtflags)
{
	while (1) {
		int fd = (*i)++;
		if (fd >= watcher->maxvisited && watcher->unvisited <= 0) break;
		*evtflags = LOSKI_EVTFLAGS_NONE;
		if (FD_ISSET(fd, &watcher->selected[0])) {
			*evtflags |= LOSKI_EVTFLAGS_INPUT;
			--watcher->unvisited;
		}
		if (FD_ISSET(fd, &watcher->selected[1])) {
			*evtflags |= LOSKI_EVTFLAGS_OUTPUT;
			--watcher->unvisited;
		}
		if (*evtflags != LOSKI_EVTFLAGS_NONE) {
			if (fd >= watcher->maxvisited) watcher->maxvisited = fd + 1;
			*src = fd;
			return 1;
		}
	}
	return 0;
}
