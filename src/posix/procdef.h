#ifndef procdef_h
#define procdef_h


#include "loskiconf.h"

#include <lua.h>
#include <sys/types.h>

typedef void loski_ProcDriver;

typedef struct loski_Process {
	pid_t pid;
	int status;
	struct loski_Process *next;
} loski_Process;

typedef char loski_ProcArgInfo;

typedef int loski_ProcEnvInfo;

typedef int loski_ProcStream;  /* file descriptor */

#define LOSKI_ENABLE_PROCFILESTREAM

LOSKIDRV_API int loskiP_luafile2stream (lua_State *L, int idx,
                                        loski_ProcStream *fd);


#endif
