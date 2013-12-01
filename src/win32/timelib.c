/*
 * Code adapted from LuaSocket 2.0.2, originally written by Diego Nehab
 *   http://w3.impa.br/~diego/software/luasocket/
 */

#include "timelib.h"

#include <windows.h>

loski_Seconds loski_now()
{
	FILETIME ft;
	double t;
	GetSystemTimeAsFileTime(&ft);
	/* Windows file time (time since January 1, 1601 (UTC)) */
	t  = ft.dwLowDateTime/1.0e7 + ft.dwHighDateTime*(4294967296.0/1.0e7);
	/* convert to Unix Epoch time (time since January 1, 1970 (UTC)) */
	return (t - 11644473600.0);
}

void loski_sleep(loski_Seconds seconds)
{
	Sleep((int)(n*1000));
}
