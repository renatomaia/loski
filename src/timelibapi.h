#ifndef timelibapi_h
#define timelibapi_h


#include "loskiconf.h"
#include "lua.h"

LOSKIDRV_API int loski_opentime(loski_TimeDriver *drv);

LOSKIDRV_API int loski_closetime(loski_TimeDriver *drv);

LOSKIDRV_API lua_Number loski_now(loski_TimeDriver *drv);

LOSKIDRV_API void loski_sleep(loski_TimeDriver *drv, lua_Number seconds);


#endif
