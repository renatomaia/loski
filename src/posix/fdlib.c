#include "luaosi/fdlib.h"

#include <errno.h>
#include <fcntl.h>

static loski_ErrorCode geterrcode ()
{
	switch (errno) {
		case EBADF: return LOSKI_ERRCLOSED;
		case EINVAL: return LOSKI_ERRINVALID;
		case EACCES:
		case EAGAIN:
		case EINTR:
		case EMFILE:
		case ENOLCK:
		case EOVERFLOW: return LOSKI_ERRUNEXPECTED;
	}
	return LOSKI_ERRUNSPECIFIED;
}

LOSKIDRV_API loski_ErrorCode loskiFD_getnonblock (int fd, int *value)
{
	int res = fcntl(fd, F_GETFL, 0);
	if (res == -1) return geterrcode();
	*value = !(res & O_NONBLOCK);
	return LOSKI_ERRNONE;
}

LOSKIDRV_API loski_ErrorCode loskiFD_setnonblock (int fd, int value)
{
	int res = fcntl(fd, F_GETFL, 0);
	if (res >= 0) {
		value = value ? (res & (~(O_NONBLOCK))) : (res | O_NONBLOCK);
		if (value != res) res = fcntl(fd, F_SETFL, value);
	}
	if (res == -1) return geterrcode();
	return LOSKI_ERRNONE;
}
