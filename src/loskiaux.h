#ifndef loskiaux_h
#define loskiaux_h


#include "loskiconf.h"
#include "loskierr.h"


#include <lua.h>
#include <lauxlib.h>


LUALIB_API void *luaL_alloctemporary(lua_State *L, size_t size);
LUALIB_API void luaL_freetemporary(lua_State *L, void *memo, size_t size);

LUALIB_API void *luaL_newsentinel(lua_State *L, size_t size, lua_CFunction f);
LUALIB_API void luaL_cancelsentinel(lua_State *L);

LUALIB_API void luaL_newclass(lua_State *L,
                              const char *name,
                              const char *super,
                              const luaL_Reg *mth,
                              int nup);
LUALIB_API int luaL_issubclass(lua_State *L, const char *cls);
LUALIB_API void *luaL_testinstance(lua_State *L, int idx, const char *cls);
LUALIB_API void *luaL_checkinstance(lua_State *L, int idx, const char *cls);

LUALIB_API void luaL_printstack(lua_State *L);

LUALIB_API void luaL_pusherrmsg(lua_State *L, loski_ErrorCode err);
LUALIB_API int luaL_doresults(lua_State *L, int nres, loski_ErrorCode err);

LOSKILIB_API int loskiU_setclassop (lua_State *L,
                                    const char *opid,
                                    const char *cls,
                                    void *func);
LOSKILIB_API void *loskiU_getvalueop (lua_State *L, const char *opid);

LUALIB_API int luaL_pushobjtab(lua_State *L, int regidx, int validx);


#endif
