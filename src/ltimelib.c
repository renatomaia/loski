#include "timelib.h"
#include "loskiaux.h"

static int lua_now(lua_State *L)
{
	loski_TimeDriver *drv = (loski_TimeDriver *)lua_touserdata(L, lua_upvalueindex(1));
	lua_pushnumber(L, loski_now(drv));
	return 1;
}

static int lua_sleep(lua_State *L)
{
	loski_TimeDriver *drv = (loski_TimeDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_sleep(drv, luaL_checknumber(L, 1));
	return 0;
}

static int lua_sentinel(lua_State *L)
{
	loski_TimeDriver *drv = (loski_TimeDriver *)lua_touserdata(L, lua_upvalueindex(1));
	loski_closetime(drv);
	return 0;
}

static const luaL_Reg lib[] = {
	{"now", lua_now},
	{"sleep", lua_sleep},
	{NULL, NULL}
};

LUAMOD_API int luaopen_time(lua_State *L)
{
	/* create sentinel */
	loski_TimeDriver *drv = (loski_TimeDriver *)luaL_newsentinel(L, sizeof(loski_TimeDriver), lua_sentinel);
	/* initialize library */
	if (loski_opentime(drv) != 0) {
		luaL_cancelsentinel(L);
		return luaL_error(L, "unable to initialize library");
	}
	/* create library table */
	luaL_newlibtable(L, lib);
	lua_pushvalue(L, -2);  /* push sentinel */
	luaL_setfuncs(L, lib, 1);
	return 1;
}
