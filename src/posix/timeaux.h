#ifndef timeaux_h
#define timeaux_h


#include "loskiconf.h"

#include <lua.h> /* seconds is a lua_Number */
#include <sys/time.h>

LOSKILIB_API void loski_seconds2timeval(lua_Number s, struct timeval *t);


#endif
