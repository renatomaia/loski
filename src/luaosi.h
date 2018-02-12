#ifndef luaosi_h
#define luaosi_h


#include "luaosi/config.h"

#include <lua.h>


#define LUA_TIMELIBNAME	"time"
LUAMOD_API int (luaopen_time) (lua_State *L);

#define LUA_PROCLIBNAME	"process"
LUAMOD_API int (luaopen_process) (lua_State *L);

#define LUA_NETLIBNAME	"network"
LUAMOD_API int (luaopen_network) (lua_State *L);

#define LUA_EVENTLIBNAME	"event"
LUAMOD_API int (luaopen_event) (lua_State *L);

#define LUA_FILESYSLIBNAME	"filesys"
LUAMOD_API int (luaopen_filesys) (lua_State *L);


/* open all previous libraries */
LOSKILIB_API void (loski_openlibs) (lua_State *L);


#endif
