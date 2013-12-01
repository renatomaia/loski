/*
 * Code adapted from LuaSocket 2.0.2, originally written by Diego Nehab
 *   http://w3.impa.br/~diego/software/luasocket/
 */

#include "timelib.h"

#include <time.h>
#include <sys/time.h>

loski_Seconds loski_now()
{
	struct timeval v;
	gettimeofday(&v, (struct timezone *) NULL);
	/* Unix Epoch time (time since January 1, 1970 (UTC)) */
	return v.tv_sec + v.tv_usec/1.0e6;
}

void loski_sleep(loski_Seconds n)
{
	struct timespec t, r;
	t.tv_sec = (int) n;
	n -= t.tv_sec;
	t.tv_nsec = (int) (n * 1000000000);
	if (t.tv_nsec >= 1000000000) t.tv_nsec = 999999999;
	while (nanosleep(&t, &r) != 0) {
		t.tv_sec = r.tv_sec;
		t.tv_nsec = r.tv_nsec;
	}
}
