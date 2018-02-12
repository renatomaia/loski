#ifndef luaosi_proclib_h
#define luaosi_proclib_h


#include "luaosi/procdef.h"

#include "luaosi/config.h"
#include "luaosi/errors.h"

/*
 * Library
 */

#ifndef LOSI_DISABLE_PROCDRV
LOSIDRV_API losi_ErrorCode losiP_initdrv (losi_ProcDriver *drv);

LOSIDRV_API void losiP_freedrv (losi_ProcDriver *drv);
#endif

/*
 * Command-Line Arguments
 */

typedef const char * (*losi_ProcArgFunc) (void *data, int index);

LOSIDRV_API int losiP_checkargs (losi_ProcDriver *drv,
                                 losi_ProcArgInfo *info,
                                 losi_ProcArgFunc getter,
                                 void *data, int count,
                                 size_t *size);

LOSIDRV_API void losiP_initargs (losi_ProcDriver *drv,
                                 losi_ProcArgInfo *info,
                                 losi_ProcArgFunc getter,
                                 void *data, int count,
                                 void *args, size_t size);

/*
 * Environment Variables
 */

typedef const char * (*losi_ProcEnvFunc) (void *data, const char **name);

LOSIDRV_API int losiP_checkenv (losi_ProcDriver *drv,
                                losi_ProcEnvInfo *info,
                                losi_ProcEnvFunc getter,
                                void *data,
                                size_t *size);

LOSIDRV_API void losiP_initenv (losi_ProcDriver *drv,
                                losi_ProcEnvInfo *info,
                                losi_ProcEnvFunc getter,
                                void *data,
                                void *envl, size_t size);

/*
 * Processes
 */

#ifndef LOSI_PROCSTAT_CUSTOM
typedef enum losi_ProcStatus {
	LOSI_PROCSTAT_RUNNING,
	LOSI_PROCSTAT_SUSPENDED,
	LOSI_PROCSTAT_DEAD
} losi_ProcStatus;
#endif

LOSIDRV_API losi_ErrorCode losiP_initproc (losi_ProcDriver *drv,
                                           losi_Process *proc,
                                           const char *binpath,
                                           const char *runpath,
                                           void *argvals,
                                           void *envlist,
                                           losi_ProcStream *stdinput,
                                           losi_ProcStream *stdoutput,
                                           losi_ProcStream *stderror);

LOSIDRV_API losi_ErrorCode losiP_getprocstat (losi_ProcDriver *drv,
                                              losi_Process *proc,
                                              losi_ProcStatus *status);

LOSIDRV_API losi_ErrorCode losiP_getprocexit (losi_ProcDriver *drv,
                                              losi_Process *proc,
                                              int *code);

LOSIDRV_API losi_ErrorCode losiP_killproc (losi_ProcDriver *drv,
                                           losi_Process *proc);

LOSIDRV_API void losiP_freeproc (losi_ProcDriver *drv,
                                 losi_Process *proc);


#endif
