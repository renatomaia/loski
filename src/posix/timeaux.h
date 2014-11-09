#ifndef timeaux_h
#define timeaux_h


#include "timelib.h"

#include <sys/time.h>

void seconds2timeval(lua_Number s, struct timeval *t);


#endif
