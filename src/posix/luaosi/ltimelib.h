#ifndef luaosi_ltimelib_h
#define luaosi_ltimelib_h


#include "luaosi/config.h"

#include <lua.h> /* seconds is a lua_Number */
#include <sys/time.h>

LOSILIB_API void losi_seconds2timeval(lua_Number s, struct timeval *t);


#endif
