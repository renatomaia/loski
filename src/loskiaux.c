#include "loskiaux.h"

#include <assert.h>
#include <string.h>

LUALIB_API void *luaL_alloctemporary(lua_State *L, size_t size)
{
	void *userdata;
	lua_Alloc alloc = lua_getallocf(L, &userdata);
	return alloc(userdata, NULL, 0, size);
}

LUALIB_API void luaL_freetemporary(lua_State *L, void *memo, size_t size)
{
	void *userdata;
	lua_Alloc alloc = lua_getallocf(L, &userdata);
	memo = alloc(userdata, memo, size, 0);
	assert(memo == NULL);
}

LUALIB_API int luaL_pushresults(lua_State *L, int nres, int err,
                                int (*pusherrmsg) (int, lua_State *)) {
	if (err) {
		int msgerr;
		lua_pushnil(L);
		msgerr = pusherrmsg(err, L);
		if (msgerr) {
			lua_pushfstring(L, "unable to get message for error code %d: ", err);
			if (pusherrmsg(msgerr, L) != 0) {
				lua_pushfstring(L, "message retrival error code %d", msgerr);
			}
			lua_concat(L, 2);
		}
		lua_pushinteger(L, err);
		nres = 3;
	} else if (nres == 0) {
		lua_pushboolean(L, 1);
		nres = 1;
	}
	return nres;
}

LUALIB_API int luaL_pushobjtab(lua_State *L, int regidx, int validx)
{
	lua_pushvalue(L, validx);    /* ...,val */
	lua_gettable(L, regidx);     /* ...,set */
	if (!lua_istable(L, -1)) {   /* ...,set */
		lua_pop(L, 1);             /* ... */
		lua_createtable(L, 0, 1);  /* ...,set */
		lua_pushvalue(L, validx);  /* ...,set,val */
		lua_pushvalue(L, -2);      /* ...,set,val,set */
		lua_settable(L, regidx);   /* ...,set */
		return 1;
	}
	return 0;
}

LUALIB_API void luaL_newsentinel(lua_State *L, lua_CFunction f)
{
	lua_newuserdata(L, 0);  /* create sentinel */
	lua_createtable(L, 0, 1);  /* create sentinel's metatable */
	lua_pushcfunction(L, f);  /* push sentinel's GC function */
	lua_setfield(L, -2, "__gc");  /* put GC function in the metatable */
	lua_setmetatable(L, -2);  /* set sentinel's metatable */
}

LUALIB_API void luaL_cancelsentinel(lua_State *L)
{
	lua_pushnil(L);
	lua_setmetatable(L, -2);  /* remove sentinel's metatable */
}

LUALIB_API void luaL_newclass(lua_State *L,
                              const char *name,
                              const luaL_Reg *mth,
                              int nup)
{
	luaL_newmetatable(L, name);  /* create metatable for file handles */
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
	lua_insert(L, -(nup+1));  /* place new metatable under the upvalues */
	luaL_setfuncs(L, mth, nup);  /* add methods to new metatable */
}

LUALIB_API void luaL_newsubclass(lua_State *L,
                                 const char *super,
                                 const char *name,
                                 const luaL_Reg *mth,
                                 int nup)
{
	static const char *const metameth[] = {"__gc", "__tostring", NULL};
	int i;
	luaL_newclass(L, name, mth, nup);
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

LUALIB_API void *luaL_testinstance(lua_State *L, int idx, const char *cls)
{
	void *p = lua_touserdata(L, idx);
	if (p != NULL) {  /* value is a userdata? */
		luaL_getmetatable(L, cls);  /* get correct metatable */
		if (lua_getmetatable(L, idx)) {  /* does it have a metatable? */
			int found = 0;
			while (!(found = lua_rawequal(L, -1, -2)) && lua_getmetatable(L, -1))
				lua_remove(L, -2);  /* remove previous metatable */
			lua_pop(L, 2);  /* remove both metatables */
			if (found)  /* some metatable was the same? */
				return p;
		}
		else lua_pop(L, 1);  /* remove correct metatable */
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
