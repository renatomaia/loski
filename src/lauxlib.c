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
		const char *typename = NULL;
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
			case LUA_TUSERDATA:
				if (lua_getmetatable(L, i)) {
					lua_getfield(L, -1, "__name");
					typename = lua_tostring(L, -1);
					lua_pop(L, 2);
				}
			default:
				printf("%s: %p", typename ? typename : luaL_typename(L, i),
				                 lua_topointer(L, i));
				break;
		}
		printf("\n");
	}
	printf("\n");
}

LUALIB_API void luaL_setclassdata (lua_State *L,
                                   const char *cls,
                                   const char *field,
                                   void *data)
{
	luaL_newmetatable(L, cls);
	lua_pushvalue(L, -1);  /* copy to be left at top */
	if (lua_gettable(L, LUA_REGISTRYINDEX) != LUA_TTABLE) {
		lua_pop(L, 1);  /* remove previous result */
		lua_newtable(L);  /* create metatable */
		lua_pushvalue(L, -2);  /* copy metatable */
		lua_pushvalue(L, -2);  /* copy op table */
		lua_settable(L, LUA_REGISTRYINDEX);  /* save op table */
	}
	if (data == NULL) lua_pushnil(L);
	else lua_pushlightuserdata(L, data);
	lua_setfield(L, -2, field);  /* save operation */
	lua_pop(L, 2);  /* removes metatable and op table */
}

LUALIB_API void *luaL_getclassdata (lua_State *L, int idx,
                                    const char *field,
                                    void **udata)
{
	*udata = lua_touserdata(L, idx);
	if (*udata != NULL && lua_getmetatable(L, idx)) {  /* udata with metatable? */
		void *func = NULL;
		if (lua_gettable(L, LUA_REGISTRYINDEX) == LUA_TTABLE) {
			if (lua_getfield(L, -1, field) == LUA_TLIGHTUSERDATA)
				func = lua_touserdata(L, -1);
			lua_pop(L, 1);  /* remove op table field */
		}
		lua_pop(L, 1);  /* remove table */
		return func;
	}
	return NULL;
}


LOSILIB_API void losiL_pusherrmsg(lua_State *L, losi_ErrorCode err)
{
	switch (err) {
		case LOSI_ERRCLOSED: lua_pushliteral(L, "closed"); break;
		case LOSI_ERRINUSE: lua_pushliteral(L, "in use"); break;
		case LOSI_ERRNOTFOUND: lua_pushliteral(L, "not found"); break;
		case LOSI_ERRUNAVAILABLE: lua_pushliteral(L, "unavailable"); break;
		case LOSI_ERRUNREACHABLE: lua_pushliteral(L, "unreachable"); break;
		case LOSI_ERRNORESOURCES: lua_pushliteral(L, "no resources"); break;
		case LOSI_ERRNOMEMORY: lua_pushliteral(L, "no system memory"); break;
		case LOSI_ERRUNFULFILLED: lua_pushliteral(L, "unfulfilled"); break;
		case LOSI_ERRTOOMUCH: lua_pushliteral(L, "too much"); break;
		case LOSI_ERRABORTED: lua_pushliteral(L, "aborted"); break;
		case LOSI_ERRREFUSED: lua_pushliteral(L, "refused"); break;
		case LOSI_ERRDENIED: lua_pushliteral(L, "access denied"); break;
		case LOSI_ERRTIMEOUT: lua_pushliteral(L, "timeout"); break;
		case LOSI_ERRSYSTEMRESET: lua_pushliteral(L, "system reset"); break;
		case LOSI_ERRSYSTEMDOWN: lua_pushliteral(L, "system down"); break;
		case LOSI_ERRSYSTEMFAIL: lua_pushliteral(L, "system error"); break;
		/* avoidable conditions */
		case LOSI_ERRINVALID: luaL_error(L, "invalid operation");
		case LOSI_ERRUNSUPPORTED: luaL_error(L, "unsupported");
		case LOSI_ERRUNEXPECTED: luaL_error(L, "unexpected error");
		case LOSI_ERRUNSPECIFIED: luaL_error(L, "unspecified error");
		default: luaL_error(L, "wrong error (%d)", err);
	}
}

LOSILIB_API int losiL_doresults(lua_State *L, int nres, losi_ErrorCode err)
{
	if (err) {
		lua_pushnil(L);
		losiL_pusherrmsg(L, err);
		nres = 2;
	} else if (nres == 0) {
		lua_pushboolean(L, 1);
		nres = 1;
	}
	return nres;
}
