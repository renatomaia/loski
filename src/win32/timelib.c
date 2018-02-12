/*
 * Code from LuaSocket 2.0.2, written by Diego Nehab
 *   http://w3.impa.br/~diego/software/luasocket/
 */

#include "timelib.h"

#include <windows.h>

lua_Number losiT_now (losi_TimeDriver *drv)
{
	FILETIME ft;
	double t;
	GetSystemTimeAsFileTime(&ft);
	/* Windows file time (time since January 1, 1601 (UTC)) */
	t  = ft.dwLowDateTime/1.0e7 + ft.dwHighDateTime*(4294967296.0/1.0e7);
	/* convert to Unix Epoch time (time since January 1, 1970 (UTC)) */
	return (t - 11644473600.0);
}

void losiT_wait (losi_TimeDriver *drv, lua_Number seconds)
{
	Sleep((int)(seconds*1000));
}
