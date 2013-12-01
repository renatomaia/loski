/*
** $Id: lualib.h,v 1.43 2011/12/08 12:11:37 roberto Exp $
** Lua operating system specific libraries
** See Copyright Notice in lua.h
*/


#ifndef luaoski_h
#define luaoski_h

#include "lua.h"



#define LUA_TIMELIBNAME	"time"
LUAMOD_API int (luaopen_time) (lua_State *L);

#define LUA_PROCLIBNAME	"process"
LUAMOD_API int (luaopen_process) (lua_State *L);

#define LUA_NETLIBNAME	"network"
LUAMOD_API int (luaopen_net) (lua_State *L);

#define LUA_EVENTLIBNAME	"event"
LUAMOD_API int (luaopen_event) (lua_State *L);

#define LUA_FILESYSLIBNAME	"filesys"
LUAMOD_API int (luaopen_filesys) (lua_State *L);


/* open all previous libraries */
LOSKILIB_API void (loski_openlibs) (lua_State *L);



#if !defined(lua_assert)
#define lua_assert(x)	((void)0)
#endif


#endif
