#ifndef luaosi_evtdef_h
#define luaosi_evtdef_h


#include "luaosi/config.h"

#include <sys/select.h>

typedef void losi_EventDriver;

typedef struct losi_EventWatcher {
	int maxfd;
	fd_set sets[2];
	/* used to iterate over results */
	fd_set selected[2];
	int maxvisited;  /* upper bound of fd iterated in selected */
	int unvisited;  /* number of fd's in selected that are >= maxvisited */
} losi_EventWatcher;

typedef int losi_EventSource;  /* file descriptor */

typedef size_t losi_EventIterator;


#endif
