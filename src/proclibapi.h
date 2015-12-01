#ifndef proclibapi_h
#define proclibapi_h

#include <loskiconf.h>

/*
 * Library
 */

#ifndef LOSKI_DISABLE_PROCDRV
LOSKIDRV_API int loskiP_initdrv (loski_ProcDriver *drv);

LOSKIDRV_API void loskiP_freedrv (loski_ProcDriver *drv);
#endif

/*
 * Command-Line Arguments
 */

typedef const char * (*loski_ProcArgFunc) (void *data, int index);

LOSKIDRV_API int loskiP_checkargs (loski_ProcDriver *drv,
                                   loski_ProcArgInfo *info,
                                   loski_ProcArgFunc getter,
                                   void *data, int count,
                                   size_t *size);

LOSKIDRV_API void loskiP_initargs (loski_ProcDriver *drv,
                                   loski_ProcArgInfo *info,
                                   loski_ProcArgFunc getter,
                                   void *data, int count,
                                   void *args, size_t size);

/*
 * Environment Variables
 */

typedef const char * (*loski_ProcEnvFunc) (void *data, const char **name);

LOSKIDRV_API int loskiP_checkenv (loski_ProcDriver *drv,
                                  loski_ProcEnvInfo *info,
                                  loski_ProcEnvFunc getter,
                                  void *data,
                                  size_t *size);

LOSKIDRV_API void loskiP_initenv (loski_ProcDriver *drv,
                                  loski_ProcEnvInfo *info,
                                  loski_ProcEnvFunc getter,
                                  void *data,
                                  void *envl, size_t size);

/*
 * Processes
 */

#include <stdio.h> /* definition of 'FILE*' */

#ifndef LOSKI_PROCSTAT_CUSTOM
typedef enum loski_ProcStatus {
	LOSKI_PROCSTAT_RUNNING,
	LOSKI_PROCSTAT_SUSPENDED,
	LOSKI_PROCSTAT_DEAD
} loski_ProcStatus;
#endif

LOSKIDRV_API int loskiP_initproc (loski_ProcDriver *drv,
                                  loski_Process *proc,
                                  const char *binpath,
                                  const char *runpath,
                                  void *argvals,
                                  void *envlist,
                                  FILE *stdinput,
                                  FILE *stdoutput,
                                  FILE *stderror);

LOSKIDRV_API int loskiP_getprocstat (loski_ProcDriver *drv,
                                     loski_Process *proc,
                                     loski_ProcStatus *status);

LOSKIDRV_API int loskiP_getprocexit (loski_ProcDriver *drv,
                                     loski_Process *proc,
                                     int *code);

LOSKIDRV_API int loskiP_killproc (loski_ProcDriver *drv,
                                  loski_Process *proc);

LOSKIDRV_API void loskiP_freeproc (loski_ProcDriver *drv,
                                   loski_Process *proc);


#endif
