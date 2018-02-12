#ifndef luaosi_procdef_h
#define luaosi_procdef_h


#include "luaosi/config.h"
#include "luaosi/errors.h"
#include "luaosi/evtlib.h"

#include <sys/types.h>

typedef void losi_ProcDriver;

typedef struct losi_Process {
	pid_t pid;
	int status;
	int pipe[2];  /* pipe for notification of child proc termination */
	size_t piperefs;
	struct losi_Process *next;
} losi_Process;

typedef char losi_ProcArgInfo;

typedef int losi_ProcEnvInfo;

typedef int losi_ProcStream;  /* file descriptor */

#define LOSI_ENABLE_LUAFILEPROCSTREAM

LOSIDRV_API int losiP_getluafileprocstrm (void *udata, losi_ProcStream *fd);

#define LOSI_ENABLE_PROCESSEVENTS

LOSIDRV_API losi_ErrorCode losiP_getprocevtsrc (void *udata, int newref,
                                                losi_EventSource *src,
                                                losi_EventFlags evtflags);
LOSIDRV_API void losiP_freeprocevtsrc (void *udata);

#endif
