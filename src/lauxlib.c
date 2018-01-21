#include "luaosi/lauxlib.h"

#include <assert.h>
#include <stdio.h>


LUALIB_API void *luaL_allocmemo(lua_State *L, size_t size)
{
	void *userdata;
	lua_Alloc alloc = lua_getallocf(L, &userdata);
	return alloc(userdata, NULL, 0, size);
}

LUALIB_API void luaL_freememo(lua_State *L, void *memo, size_t size)
{
	void *userdata;
	lua_Alloc alloc = lua_getallocf(L, &userdata);
	memo = alloc(userdata, memo, size, 0);
	assert(memo == NULL);
}


LUALIB_API void *luaL_newsentinel(lua_State *L, size_t size, lua_CFunction f)
{
	void *state = lua_newuserdata(L, size);  /* create sentinel */
	lua_createtable(L, 0, 1);  /* create sentinel's metatable */
	lua_pushcfunction(L, f);  /* push sentinel's GC function */
	lua_setfield(L, -2, "__gc");  /* put GC function in the metatable */
	lua_setmetatable(L, -2);  /* set sentinel's metatable */
	return state;
}

LUALIB_API void luaL_cancelsentinel(lua_State *L)
{
	lua_pushnil(L);
	lua_setmetatable(L, -2);  /* remove sentinel's metatable */
}


LUALIB_API void luaL_newclass(lua_State *L,
                              const char *name,
                              const char *super,
                              const luaL_Reg *mth,
                              int nup)
{
	static const char *const metameth[] = {"__gc", "__tostring", NULL};
	int i;
	luaL_newmetatable(L, name);  /* create metatable for instances */
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
	lua_insert(L, -(nup+1));  /* place new metatable under the upvalues */
	luaL_setfuncs(L, mth, nup);  /* add methods to new metatable */
	if (super) {
		luaL_getmetatable(L, super);
		for (i=0; metameth[i]; ++i) {
			lua_getfield(L, -2, metameth[i]);  /* get metamethod from subclass */
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);  /* remove nil */
				lua_getfield(L, -1, metameth[i]);  /* get metamethod from superclass */
				lua_setfield(L, -3, metameth[i]);  /* set metamethod of subclass */
			}
		}
		lua_setmetatable(L, -2);
	}
}

LUALIB_API int luaL_issubclass(lua_State *L, const char *cls)
{
	int found = 0;
	luaL_getmetatable(L, cls);  /* get expected metatable */
	lua_pushvalue(L, -2);  /* get the value */
	while (!(found = lua_rawequal(L, -1, -2)) && lua_getmetatable(L, -1))
		lua_remove(L, -2);  /* remove previous metatable */
	lua_pop(L, 2);  /* remove both metatables */
	return found;
}

LUALIB_API void *luaL_testinstance(lua_State *L, int idx, const char *cls)
{
	void *p = lua_touserdata(L, idx);
	if (p != NULL) {  /* value is a userdata? */
		if (lua_getmetatable(L, idx)) {  /* does it have a metatable? */
			if (!luaL_issubclass(L, cls)) p = NULL;
			lua_pop(L, 1);  /* remove the metatable */
			return p;
		}
	}
	return NULL;  /* value is not a userdata with a metatable */
}

LUALIB_API void *luaL_checkinstance(lua_State *L, int idx, const char *cls)
{
	void *p = luaL_testinstance(L, idx, cls);
	if (p == NULL) {
		const char *msg = lua_pushfstring(L, "%s expected, got %s",
		                                  cls, luaL_typename(L, idx));
		luaL_argerror(L, idx, msg);
		return NULL;
	}
	return p;
}


LUALIB_API void luaL_printstack(lua_State *L)
{
	int i;
	for(i = 1; i <= lua_gettop(L); ++i) {
		printf("%d = ", i);
		switch (lua_type(L, i)) {
			case LUA_TNUMBER:
				printf("%g", lua_tonumber(L, i));
				break;
			case LUA_TSTRING:
				printf("\"%s\"", lua_tostring(L, i));
				break;
			case LUA_TBOOLEAN:
				printf(lua_toboolean(L, i) ? "true" : "false");
				break;
			case LUA_TNIL:
				printf("nil");
				break;
			default:
				printf("%s: %p", luaL_typename(L, i),
				                 lua_topointer(L, i));
				break;
		}
		printf("\n");
	}
	printf("\n");
}


LOSKILIB_API void loskiL_pusherrmsg(lua_State *L, loski_ErrorCode err)
{
	switch (err) {
		case LOSKI_ERRCLOSED: lua_pushliteral(L, "closed"); break;
		case LOSKI_ERRINUSE: lua_pushliteral(L, "in use"); break;
		case LOSKI_ERRNOTFOUND: lua_pushliteral(L, "not found"); break;
		case LOSKI_ERRUNAVAILABLE: lua_pushliteral(L, "unavailable"); break;
		case LOSKI_ERRUNREACHABLE: lua_pushliteral(L, "unreachable"); break;
		case LOSKI_ERRNORESOURCES: lua_pushliteral(L, "no resources"); break;
		case LOSKI_ERRNOMEMORY: lua_pushliteral(L, "no system memory"); break;
		case LOSKI_ERRUNFULFILLED: lua_pushliteral(L, "unfulfilled"); break;
		case LOSKI_ERRTOOMUCH: lua_pushliteral(L, "too much"); break;
		case LOSKI_ERRABORTED: lua_pushliteral(L, "aborted"); break;
		case LOSKI_ERRREFUSED: lua_pushliteral(L, "refused"); break;
		case LOSKI_ERRDENIED: lua_pushliteral(L, "access denied"); break;
		case LOSKI_ERRTIMEOUT: lua_pushliteral(L, "timeout"); break;
		case LOSKI_ERRSYSTEMRESET: lua_pushliteral(L, "system reset"); break;
		case LOSKI_ERRSYSTEMDOWN: lua_pushliteral(L, "system down"); break;
		case LOSKI_ERRSYSTEMFAIL: lua_pushliteral(L, "system error"); break;
		/* avoidable conditions */
		case LOSKI_ERRINVALID: luaL_error(L, "invalid operation");
		case LOSKI_ERRUNSUPPORTED: luaL_error(L, "unsupported");
		case LOSKI_ERRUNEXPECTED: luaL_error(L, "unexpected error");
		case LOSKI_ERRUNSPECIFIED: luaL_error(L, "unspecified error");
		default: luaL_error(L, "wrong error (%d)", err);
	}
}

LOSKILIB_API int loskiL_doresults(lua_State *L, int nres, loski_ErrorCode err)
{
	if (err) {
		lua_pushnil(L);
		loskiL_pusherrmsg(L, err);
		nres = 2;
	} else if (nres == 0) {
		lua_pushboolean(L, 1);
		nres = 1;
	}
	return nres;
}


LOSKILIB_API int loskiL_setclassop (lua_State *L,
                                    const char *opid,
                                    const char *cls,
                                    void *func)
{
	luaL_getsubtable(L, LUA_REGISTRYINDEX, opid);
	if (luaL_getmetatable(L, cls) == LUA_TTABLE) {
		if (func == NULL) lua_pushnil(L);
		else lua_pushlightuserdata(L, func);
		lua_settable(L, -3);
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 2);
	return 0;
}

LOSKILIB_API void *loskiL_getvalueop (lua_State *L, const char *opid)
{
	void *func = NULL;
	int idx = lua_gettop(L);
	if ((lua_getfield(L, LUA_REGISTRYINDEX, opid) == LUA_TTABLE) &&
	    (lua_getmetatable(L, idx)) &&
	    (lua_gettable(L, -2) == LUA_TLIGHTUSERDATA))
		func = lua_touserdata(L, -1);
	lua_settop(L, idx);
	return func;
}
