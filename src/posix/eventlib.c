#include "eventlib.h"
#include "timeaux.h"

#include <errno.h>
#include <sys/select.h>
#include <lua.h> /* to copy error messages to Lua */

#define UNWATCHABLE_OBJECT_ERROR (-1)

LOSKIDRV_API int loski_openevents(loski_EventDriver *drv)
{
	return 0;
}

LOSKIDRV_API int loski_closeevents(loski_EventDriver *drv)
{
	return 0;
}

LOSKIDRV_API int loski_eventerror(int error, lua_State* L)
{
	switch (error) {
	case UNWATCHABLE_OBJECT_ERROR:
		lua_pushliteral(L, "unwatchable");
		break;
	default:
		lua_pushstring(L, strerror(error));
		break;
	}
	return 0;
}

LOSKIDRV_API int loski_initwatcher(loski_EventDriver *drv,
                                   loski_EventWatcher *watcher)
{
	FD_ZERO(&watcher->sets[0]);
	FD_ZERO(&watcher->sets[1]);
	return 0;
}

LOSKIDRV_API int loski_endwatcher(loski_EventDriver *drv,
                                  loski_EventWatcher *watcher)
{
	return 0;
}

LOSKIDRV_API void loski_pushwachedkey(lua_State *L,
                                      loski_EventWatch *watch)
{
	switch (watch->kind) {
		case LOSKI_WATCHFILE:
			lua_pushnumber(L, watch->object.file);
			break;
		default:
			lua_pushnil(L);
			break;
	}
}

LOSKIDRV_API int loski_addwatch(loski_EventDriver *drv,
                                loski_EventWatcher *watcher,
                                loski_EventWatch *watch,
                                loski_WatchableReference *ref)
{
	int fd;
	switch (watch->kind) {
		case LOSKI_WATCHSOCKET:
			fd = *(ref->socket);
			watch->object.file = fd;
			watch->kind = LOSKI_WATCHFILE;
			break;
		default:
			return UNWATCHABLE_OBJECT_ERROR;
	}
	FD_SET(fd, &watcher->sets[watch->event]);
	if (fd >= watcher->maxfd) watcher->maxfd = fd+1;
	return 0;
}

LOSKIDRV_API int loski_delwatch(loski_EventDriver *drv,
                                loski_EventWatcher *watcher,
                                loski_EventWatch *watch,
                                loski_WatchableReference *ref)
{
	int fd, i;
	switch (watch->kind) {
		case LOSKI_WATCHSOCKET:
			fd = *(ref->socket);
			watch->object.file = fd;
			watch->kind = LOSKI_WATCHFILE;
			break;
		default:
			return UNWATCHABLE_OBJECT_ERROR;
	}
	FD_CLR(fd, &watcher->sets[watch->event]);
	if (fd+1 == watcher->maxfd) {
		for (i=fd-1; i>=0; --i)
			if (FD_ISSET(i, &watcher->sets[watch->event]))
				break;
		watcher->maxfd = i+1;
	}
	return 0;
}

LOSKIDRV_API size_t loski_eventqueuesize(loski_EventDriver *drv,
                                         loski_EventWatcher *watcher,
                                         size_t count)
{
	return 2*sizeof(fd_set);
}

LOSKIDRV_API int loski_waitevent(loski_EventDriver *drv,
                                 loski_EventWatcher *watcher,
                                 void *queue,
                                 size_t *count,
                                 lua_Number timeout)
{
	int res;
	fd_set *sets = (fd_set *)queue;
	memcpy(&sets[0], &watcher->sets[0], sizeof(fd_set));
	memcpy(&sets[1], &watcher->sets[1], sizeof(fd_set));
	if (timeout >= 0) {
		struct timeval tm;
		seconds2timeval(timeout, &tm);
		res = select(watcher->maxfd, &sets[0], &sets[1], NULL, &tm);
	} else {
		res = select(watcher->maxfd, &sets[0], &sets[1], NULL, NULL);
	}
	if (res == -1) return errno;
	*count = 2*(watcher->maxfd);
	return 0;
}

LOSKIDRV_API int loski_getevent(loski_EventDriver *drv,
                                loski_EventWatch *watch,
                                void *queue,
                                size_t index)
{
	fd_set *sets = (fd_set *)queue;
	int si = index%2;
	int fd = (index-si)/2;
	if (FD_ISSET(fd, &sets[si])) {
		watch->kind = LOSKI_WATCHFILE;
		watch->object.file = fd;
		watch->event = si;
		return 0;
	}
	return 1;
}
