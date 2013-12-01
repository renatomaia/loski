#ifndef proclibapi_h
#define proclibapi_h

#include "loskiconf.h"

#include <stdio.h>

typedef struct loski_Process loski_Process;

enum loski_ProcStatus { LOSKI_RUNNINGPROC, LOSKI_SUSPENDEDPROC, LOSKI_DEADPROC };
typedef enum loski_ProcStatus loski_ProcStatus;

LOSKIDRV_API int loski_openprocesses();

LOSKIDRV_API int loski_closeprocesses();

LOSKIDRV_API const char *loski_processerror(int error);

LOSKIDRV_API int loski_createprocess(loski_Process *proc,
                                   const char *binpath,
                                   const char *runpath,
                                   char *const argvals[],
                                   char *const envlist[],
                                   FILE *stdin,
                                   FILE *stdout,
                                   FILE *stderr);

LOSKIDRV_API int loski_processstatus(loski_Process *proc,
                                   loski_ProcStatus *status);

LOSKIDRV_API int loski_processexitval(loski_Process *proc, int *code);

LOSKIDRV_API int loski_killprocess(loski_Process *proc);

LOSKIDRV_API int loski_discardprocess(loski_Process *proc);


#endif
