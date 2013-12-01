#ifndef timelib_h
#define timelib_h

#include "loskiconf.h"

typedef double loski_Seconds;

LOSKIDRV_API loski_Seconds loski_now();

LOSKIDRV_API void loski_sleep(loski_Seconds seconds);


#endif
