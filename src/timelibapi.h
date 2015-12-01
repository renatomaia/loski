#ifndef timelibapi_h
#define timelibapi_h

#include <loskiconf.h>

#include <lua.h> /* seconds is a lua_Number */

#ifndef LOSKI_DISABLE_TIMEDRV
LOSKIDRV_API int loskiT_initdrv (loski_TimeDriver *drv);

LOSKIDRV_API int loskiT_freedrv (loski_TimeDriver *drv);
#endif

LOSKIDRV_API lua_Number loskiT_now (loski_TimeDriver *drv);

LOSKIDRV_API void loskiT_wait (loski_TimeDriver *drv, lua_Number seconds);


#endif
