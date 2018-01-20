#include "loskiaux.h"

#include <timelib.h>


#ifdef LOSKI_DISABLE_TIMEDRV
#define DRVUPV	0
#define todrv(L)	NULL
#else
#define DRVUPV	1
#define todrv(L)  ((loski_TimeDriver *)lua_touserdata(L, \
                                       lua_upvalueindex(DRVUPV)))
#endif


static int lua_now (lua_State *L)
{
	loski_TimeDriver *drv = todrv(L);
	lua_pushnumber(L, loskiT_now(drv));
	return 1;
}

static int lua_sleep (lua_State *L)
{
	loski_TimeDriver *drv = todrv(L);
	loskiT_wait(drv, luaL_checknumber(L, 1));
	return 0;
}


static const luaL_Reg lib[] = {
	{"now", lua_now},
	{"sleep", lua_sleep},
	{NULL, NULL}
};

#ifndef LOSKI_DISABLE_TIMEDRV
static int lfreedrv (lua_State *L)
{
	loskiT_freedrv((loski_TimeDriver *)lua_touserdata(L, 1));
	return 0;
}
#endif

LUAMOD_API int luaopen_time (lua_State *L)
{
#ifndef LOSKI_DISABLE_TIMEDRV
	/* create sentinel */
	lua_settop(L, 0);  /* dicard any arguments */
	drv = (loski_TimeDriver *)luaL_newsentinel(L, sizeof(loski_TimeDriver),
	                                              lfreedrv);
	/* initialize library */
	err = loskiT_initdrv(drv)
	if (err) {
		luaL_cancelsentinel(L);
		loskiL_pusherrmsg(L, err);
		return lua_error(L);
	}
#define pushsentinel(L)	lua_pushvalue(L, 1)
#else
#define pushsentinel(L)	((void)L)
#endif
	/* create library table */
	luaL_newlibtable(L, lib);
	pushsentinel(L);
	luaL_setfuncs(L, lib, DRVUPV);
	return 1;
}

