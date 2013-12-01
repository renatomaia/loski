#include "eventlib.h"
#include "timeaux.h"

#include <errno.h>
#include <sys/select.h>

#define UNWATCHABLE_OBJECT_ERROR (-1)

LOSKIDRV_API int loski_openevents()
{
	return 0;
}

LOSKIDRV_API int loski_closeevents()
{
	return 0;
}

LOSKIDRV_API const char *loski_eventerror(int error)
{
	switch (error) {
		case UNWATCHABLE_OBJECT_ERROR: return "unwatchable";
	}
	return strerror(error);
}

LOSKIDRV_API int loski_initwatcher(loski_EventWatcher *watcher)
{
	FD_ZERO(&watcher->sets[0]);
	FD_ZERO(&watcher->sets[1]);
	return 0;
}

LOSKIDRV_API int loski_endwatcher(loski_EventWatcher *watcher)
{
	return 0;
}

LOSKIDRV_API int loski_addwatch(loski_EventWatcher *watcher,
                              loski_EventWatch *watch)
{
	int fd;
	switch (watch->kind) {
		case LOSKI_WATCHSOCKET:
			fd = *(watch->object.socket);
			break;
		default:
			return UNWATCHABLE_OBJECT_ERROR;
	}
	FD_SET(fd, &watcher->sets[watch->event]);
	if (fd >= watcher->maxfd) watcher->maxfd = fd+1;
	return 0;
}

LOSKIDRV_API int loski_delwatch(loski_EventWatcher *watcher,
                              loski_EventWatch *watch)
{
	int fd, i;
	switch (watch->kind) {
		case LOSKI_WATCHSOCKET:
			fd = *(watch->object.socket);
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

LOSKIDRV_API size_t loski_eventqueuesize(loski_EventWatcher *watcher,
                                       size_t count)
{
	return 2*sizeof(fd_set);
}

LOSKIDRV_API int loski_waitevent(loski_EventWatcher *watcher,
                               void *queue,
                               size_t *count,
                               loski_Seconds timeout)
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

LOSKIDRV_API int loski_getevent(void *queue,
                              size_t index,
                              loski_EventWatch *watch)
{
	fd_set *sets = (fd_set *)queue;
	int si = index%2;
	int fd = (index-si)/2;
	if (FD_ISSET(fd, &sets[si])) {
		watch->kind = LOSKI_WATCHSOCKET;
		*watch->object.socket = fd;
		watch->event = si;
		return 0;
	}
	return 1;
}
