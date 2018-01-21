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
	FD_ZERO(&watcher->sets[LOSKI_EVTVAL_INPUT]);
	FD_ZERO(&watcher->sets[LOSKI_EVTVAL_OUTPUT]);
	return LOSKI_ERRNONE;
}

LOSKIDRV_API loski_ErrorCode loskiE_endwatcher(loski_EventDriver *drv,
                                               loski_EventWatcher *watcher)
{
	return LOSKI_ERRNONE;
}

LOSKIDRV_API loski_ErrorCode loskiE_addwatch(loski_EventDriver *drv,
                                             loski_EventWatcher *watcher,
                                             loski_EventSource *source,
                                             loski_EventValue event)
{
	if (*source < 0 || *source >= FD_SETSIZE) return LOSKI_ERRINVALID;
	FD_SET(*source, &watcher->sets[event]);
	if (*source >= watcher->maxfd) watcher->maxfd = *source+1;
	return LOSKI_ERRNONE;
}

LOSKIDRV_API loski_ErrorCode loskiE_delwatch(loski_EventDriver *drv,
                                             loski_EventWatcher *watcher,
                                             loski_EventSource *source,
                                             loski_EventValue event)
{
	if (*source < 0 || *source >= FD_SETSIZE) return LOSKI_ERRINVALID;
	FD_CLR(*source, &watcher->sets[event]);
	if (*source+1 == watcher->maxfd) {
		int i;
		for (i=*source-1; i>=0; --i)
			if (FD_ISSET(i, &watcher->sets[LOSKI_EVTVAL_INPUT])
			||  FD_ISSET(i, &watcher->sets[LOSKI_EVTVAL_OUTPUT]))
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
	watcher->selected[LOSKI_EVTVAL_INPUT] = watcher->sets[LOSKI_EVTVAL_INPUT];
	watcher->selected[LOSKI_EVTVAL_OUTPUT] = watcher->sets[LOSKI_EVTVAL_OUTPUT];
	if (timeout >= 0) {
		struct timeval tm;
		loski_seconds2timeval(timeout, &tm);
		watcher->unvisited = select(watcher->maxfd,
		                            &watcher->selected[LOSKI_EVTVAL_INPUT],
		                            &watcher->selected[LOSKI_EVTVAL_OUTPUT],
		                            NULL, &tm);
	} else {
		watcher->unvisited = select(watcher->maxfd,
		                            &watcher->selected[LOSKI_EVTVAL_INPUT],
		                            &watcher->selected[LOSKI_EVTVAL_OUTPUT],
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
                                  loski_EventValue *event)
{
	while (1) {
		int setidx = *index%2;
		int fd = (*index-setidx)/2;
		if (fd >= watcher->maxvisited && watcher->unvisited == 0)
			break;
		else if (FD_ISSET(fd, &watcher->selected[setidx])) {
			if (fd >= watcher->maxvisited) {
				watcher->maxvisited = fd + 1;
				--watcher->unvisited;
			}
			*source = fd;
			*event = setidx;
			return 1;
		}
		++*index;
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
