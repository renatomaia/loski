#ifndef luaosi_timelib_h
#define luaosi_timelib_h


#include "luaosi/timedef.h"
#include "luaosi/config.h"

#include <lua.h> /* seconds is a lua_Number */

#ifndef LOSI_DISABLE_TIMEDRV
LOSIDRV_API losi_ErrorCode losiT_initdrv (losi_TimeDriver *drv);

LOSIDRV_API void losiT_freedrv (losi_TimeDriver *drv);
#endif

LOSIDRV_API lua_Number losiT_now (losi_TimeDriver *drv);

LOSIDRV_API void losiT_wait (losi_TimeDriver *drv, lua_Number seconds);


#endif
