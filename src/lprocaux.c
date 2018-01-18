#include "lprocaux.h"

#include <lauxlib.h>


LOSKILIB_API int loski_isprocess (lua_State *L, int idx)
{
	return luaL_testudata(L, idx, LOSKI_PROCESSCLS) != NULL;
}

LOSKILIB_API loski_Process *loski_toprocess (lua_State *L, int idx)
{
	if (!loski_isprocess(L, idx)) return NULL;
	return (loski_Process *)lua_touserdata(L, idx);
}
