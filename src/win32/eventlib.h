#ifndef eventlib_h
#define eventlib_h


#include <winsock.h>
#include <string.h>

typedef struct loski_EventWatcher {
	int maxfd;
	fd_set sets[2];
} loski_EventWatcher;

typedef int loski_WatchableFile;
typedef int loski_WatchableSocket;
typedef int loski_WatchableProcess;


#include "eventlibapi.h"


#endif
