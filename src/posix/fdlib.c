#include "luaosi/fdlib.h"

#include <errno.h>
#include <fcntl.h>

static losi_ErrorCode geterrcode ()
{
	switch (errno) {
		case EBADF: return LOSI_ERRCLOSED;
		case EINVAL: return LOSI_ERRINVALID;
		case EACCES:
		case EAGAIN:
		case EINTR:
		case EMFILE:
		case ENOLCK:
		case EOVERFLOW: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiFD_getnonblock (int fd, int *value)
{
	int res = fcntl(fd, F_GETFL, 0);
	if (res == -1) return geterrcode();
	*value = !(res & O_NONBLOCK);
	return LOSI_ERRNONE;
}

LOSIDRV_API losi_ErrorCode losiFD_setnonblock (int fd, int value)
{
	int res = fcntl(fd, F_GETFL, 0);
	if (res >= 0) {
		value = value ? (res & (~(O_NONBLOCK))) : (res | O_NONBLOCK);
		if (value != res) res = fcntl(fd, F_SETFL, value);
	}
	if (res == -1) return geterrcode();
	return LOSI_ERRNONE;
}
