#ifndef lnetaux_h
#define lnetaux_h


#include "lnetlib.h"
#include "netlib.h"

#include <lua.h>


LOSKILIB_API int loski_issocket (lua_State *L, int idx, int cls);
LOSKILIB_API loski_Socket *loski_tosocket (lua_State *L, int idx, int cls);


#endif
