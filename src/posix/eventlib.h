#ifndef eventlib_h
#define eventlib_h


#include <sys/select.h>
#include <string.h>

typedef struct loski_EventWatcher {
	int maxfd;
	fd_set sets[2];
} loski_EventWatcher;


#include "eventlibapi.h"


#endif
