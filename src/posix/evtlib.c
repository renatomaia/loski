#include "luaosi/evtlib.h"


#include "luaosi/levtlib.h"
#include "luaosi/ltimelib.h"

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
                                             loski_EventSource *source,
                                             loski_EventFlags evtflags)
{
	if (*source < 0 || *source >= FD_SETSIZE) return LOSKI_ERRINVALID;
	if (evtflags & LOSKI_EVTFLAGS_INPUT) FD_SET(*source, &watcher->sets[0]);
	else                                 FD_CLR(*source, &watcher->sets[0]);
	if (evtflags & LOSKI_EVTFLAGS_OUTPUT) FD_SET(*source, &watcher->sets[1]);
	else                                  FD_CLR(*source, &watcher->sets[1]);
	if (evtflags != LOSKI_EVTFLAGS_NONE) {
		if (*source >= watcher->maxfd) watcher->maxfd = *source+1;
	} else if (*source+1 == watcher->maxfd) {
		int i;
		for (i=*source-1; i>=0; --i)
			if (FD_ISSET(i, &watcher->sets[0]) || FD_ISSET(i, &watcher->sets[1]))
				break;
		watcher->maxfd = i+1;
	}
	return LOSKI_ERRNONE;
}

LOSKIDRV_API loski_IntUniqueId loskiE_getsourceid(loski_EventDriver *drv,
                                                  loski_EventSource *source)
{
	return (loski_IntUniqueId)(*source);
}

LOSKIDRV_API loski_ErrorCode loskiE_waitevent(loski_EventDriver *drv,
                                              loski_EventWatcher *watcher,
                                              lua_Number timeout)
{
	watcher->maxvisited = 0;
	watcher->selected[0] = watcher->sets[0];
	watcher->selected[1] = watcher->sets[1];
	if (timeout >= 0) {
		struct timeval tm;
		loski_seconds2timeval(timeout, &tm);
		watcher->unvisited = select(watcher->maxfd,
		                            &watcher->selected[0],
		                            &watcher->selected[1],
		                            NULL, &tm);
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
		case EINTR: return LOSKI_ERRUNFULFILLED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API int loskiE_nextevent(loski_EventDriver *drv,
                                  loski_EventWatcher *watcher,
                                  size_t *index,
                                  loski_EventSource *source,
                                  loski_EventFlags *evtflags)
{
	while (1) {
		int fd = (int)*index;
		if (fd >= watcher->maxvisited && watcher->unvisited <= 0) break;
		++(*index);
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
			*source = fd;
			return 1;
		}
	}
	return 0;
}

LOSKIDRV_API int loskiE_luafile2evtsrc (lua_State *L, int idx,
                                        loski_EventSource *fd)
{
	FILE **fp = (FILE **)luaL_testudata(L, idx, LUA_FILEHANDLE);
	if (*fp == NULL) return LOSKI_ERRNONE;
	*fd = fileno(*fp);
	return 1;
}
