/*
 * Code adapted from LuaSocket 2.0.2, originally written by Diego Nehab
 *   http://w3.impa.br/~diego/software/luasocket/
 */

#include "timeaux.h"

#define INTSEC_MAX (1.0e6)

void seconds2timeval(loski_Seconds s, struct timeval *t)
{
	t->tv_sec = (int) s;
	t->tv_usec = (int) ((s - t->tv_sec) * INTSEC_MAX);
}
