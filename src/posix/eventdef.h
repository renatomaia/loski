#ifndef eventdef_h
#define eventdef_h


#include "loskiconf.h"

#include <lua.h>
#include <sys/select.h>
#include <string.h>

typedef void loski_EventDriver;
typedef struct loski_EventWatcher {
	int maxfd;
	fd_set sets[2];
	/* used to iterate over results */
	fd_set selected[2];
	int maxvisited;  /* upper bound of fd iterated in selected */
	int unvisited;  /* number of fd's in selected that are >= maxvisited */
} loski_EventWatcher;
typedef int loski_EventSource;

#define LOSKI_ENABLE_LUAFILEEVENTS

LOSKIDRV_API int loskiE_luafile2evtsrc (lua_State *L, int idx,
                                        loski_EventSource *fd);


#endif
