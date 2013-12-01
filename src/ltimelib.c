#include "timelib.h"
#include "lauxlib.h"

static int lua_now(lua_State *L)
{
	lua_pushnumber(L, loski_now());
	return 1;
}

static int lua_sleep(lua_State *L)
{
	loski_sleep(luaL_checknumber(L, 1));
	return 0;
}

static const luaL_Reg lib[] = {
	{"now", lua_now},
	{"sleep", lua_sleep},
	{NULL, NULL}
};

LUAMOD_API int luaopen_time(lua_State *L)
{
	luaL_newlib(L, lib);
	return 1;
}
