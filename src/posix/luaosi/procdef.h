#ifndef luaosi_procdef_h
#define luaosi_procdef_h


#include "luaosi/config.h"
#include "luaosi/errors.h"
#include "luaosi/evtlib.h"

#include <sys/types.h>

typedef void loski_ProcDriver;

typedef struct loski_Process {
	pid_t pid;
	int status;
	int pipe[2];  /* pipe for notification of child proc termination */
	size_t piperefs;
	struct loski_Process *next;
} loski_Process;

typedef char loski_ProcArgInfo;

typedef int loski_ProcEnvInfo;

typedef int loski_ProcStream;  /* file descriptor */

#define LOSKI_ENABLE_LUAFILEPROCSTREAM

LOSKIDRV_API int loskiP_getluafileprocstrm (void *udata, loski_ProcStream *fd);

#define LOSKI_ENABLE_PROCESSEVENTS

LOSKIDRV_API loski_ErrorCode loskiP_getprocevtsrc (void *udata, int newref,
                                                   loski_EventSource *src,
                                                   loski_EventFlags evtflags);
LOSKIDRV_API void loskiP_freeprocevtsrc (void *udata);

#endif
