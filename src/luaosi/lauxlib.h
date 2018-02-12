#ifndef luaosi_lauxlib_h
#define luaosi_lauxlib_h


#include <lua.h>
#include <lauxlib.h>


LUALIB_API void *luaL_allocmemo(lua_State *L, size_t size);
LUALIB_API void luaL_freememo(lua_State *L, void *memo, size_t size);

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

LUALIB_API void luaL_setclassdata (lua_State *L,
                                   const char *cls,
                                   const char *field,
                                   void *data);
LUALIB_API void *luaL_getclassdata (lua_State *L, int idx,
                                    const char *field,
                                    void **udata);


#include "luaosi/config.h"
#include "luaosi/errors.h"


LOSILIB_API void losiL_pusherrmsg(lua_State *L, losi_ErrorCode err);
LOSILIB_API int losiL_doresults(lua_State *L, int nres, losi_ErrorCode err);


#endif
