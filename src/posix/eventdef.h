#ifndef eventdef_h
#define eventdef_h


#include <sys/select.h>
#include <string.h>

typedef void loski_EventDriver;
typedef struct loski_EventWatcher {
	int maxfd;
	fd_set sets[2];
} loski_EventWatcher;
typedef int loski_WatchableFile;
typedef int loski_WatchableSocket;
typedef int loski_WatchableProcess;


#endif
