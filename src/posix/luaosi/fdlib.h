#ifndef luaosi_fdlib_h
#define luaosi_fdlib_h


#include "luaosi/config.h"
#include "luaosi/errors.h"

LOSKIDRV_API loski_ErrorCode loskiFD_getnonblock (int fd, int *value);
LOSKIDRV_API loski_ErrorCode loskiFD_setnonblock (int fd, int value);


#endif
