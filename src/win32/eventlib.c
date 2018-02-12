#include "eventlib.h"
#include "timeaux.h"

#include <lua.h> /* to copy error messages to Lua */

#define UNWATCHABLE_OBJECT_ERROR (-1)

LOSIDRV_API int losi_openevents(losi_EventDriver *drv)
{
	return 0;
}

LOSIDRV_API int losi_closeevents(losi_EventDriver *drv)
{
	return 0;
}

LOSIDRV_API int losi_eventerror(int error, lua_State* L)
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

LOSIDRV_API int losi_initwatcher(losi_EventDriver *drv,
                                   losi_EventWatcher *watcher)
{
	FD_ZERO(&watcher->sets[0]);
	FD_ZERO(&watcher->sets[1]);
	return 0;
}

LOSIDRV_API int losi_endwatcher(losi_EventDriver *drv,
                                  losi_EventWatcher *watcher)
{
	return 0;
}

LOSIDRV_API void losi_pushwachedkey(lua_State *L,
                                      losi_EventWatch *watch)
{
	switch (watch->kind) {
		case LOSI_WATCHFILE:
			lua_pushnumber(L, watch->object.file);
			break;
		default:
			lua_pushnil(L);
			break;
	}
}

LOSIDRV_API int losi_addwatch(losi_EventDriver *drv,
                                losi_EventWatcher *watcher,
                                losi_EventWatch *watch,
                                losi_WatchableReference *ref)
{
	int fd;
	switch (watch->kind) {
		case LOSI_WATCHSOCKET:
			fd = ref->socket->id;
			watch->object.file = fd;
			watch->kind = LOSI_WATCHFILE;
			break;
		default:
			return UNWATCHABLE_OBJECT_ERROR;
	}
	FD_SET(fd, &watcher->sets[watch->event]);
	if (fd >= watcher->maxfd) watcher->maxfd = fd+1;
	return 0;
}

LOSIDRV_API int losi_delwatch(losi_EventDriver *drv,
                                losi_EventWatcher *watcher,
                                losi_EventWatch *watch,
                                losi_WatchableReference *ref)
{
	int fd, i;
	switch (watch->kind) {
		case LOSI_WATCHSOCKET:
			fd = ref->socket->id;
			watch->object.file = fd;
			watch->kind = LOSI_WATCHFILE;
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

LOSIDRV_API size_t losi_eventqueuesize(losi_EventDriver *drv,
                                         losi_EventWatcher *watcher,
                                         size_t count)
{
	return 2*sizeof(fd_set);
}

LOSIDRV_API int losi_waitevent(losi_EventDriver *drv,
                                 losi_EventWatcher *watcher,
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
		res = select(0, &sets[0], &sets[1], NULL, &tm);
	} else {
		res = select(0, &sets[0], &sets[1], NULL, NULL);
	}
	if (res == SOCKET_ERROR) return WSAGetLastError();
	*count = (res == 0) ? 0 : 2*(watcher->maxfd);
	return 0;
}

LOSIDRV_API int losi_getevent(losi_EventDriver *drv,
                                losi_EventWatch *watch,
                                void *queue,
                                size_t index)
{
	fd_set *sets = (fd_set *)queue;
	int si = index%2;
	int fd = (index-si)/2;
	if (FD_ISSET(fd, &sets[si])) {
		watch->kind = LOSI_WATCHFILE;
		watch->object.file = fd;
		watch->event = si;
		return 0;
	}
	return 1;
}
