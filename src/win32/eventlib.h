#ifndef eventlib_h
#define eventlib_h


#include <winsock.h>
#include <string.h>

typedef void losi_EventDriver;
typedef struct losi_EventWatcher {
	int maxfd;
	fd_set sets[2];
} losi_EventWatcher;
typedef int losi_WatchableFile;
typedef int losi_WatchableSocket;
typedef int losi_WatchableProcess;


#include "eventlibapi.h"


#endif
