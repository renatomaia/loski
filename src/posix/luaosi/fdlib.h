#ifndef luaosi_fdlib_h
#define luaosi_fdlib_h


#include "luaosi/config.h"
#include "luaosi/errors.h"

LOSIDRV_API losi_ErrorCode losiFD_getnonblock (int fd, int *value);
LOSIDRV_API losi_ErrorCode losiFD_setnonblock (int fd, int value);


#endif
