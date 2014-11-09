#ifndef proclibapi_h
#define proclibapi_h

#include "loskiconf.h"

#include <stdio.h> /* definition of 'FILE*' */
#include <lua.h>

typedef const char * (*loski_ProcArgFunc) (lua_State *L, size_t index);

enum loski_ProcStatus { LOSKI_RUNNINGPROC, LOSKI_SUSPENDEDPROC, LOSKI_DEADPROC };
typedef enum loski_ProcStatus loski_ProcStatus;

LOSKIDRV_API int loski_openprocesses(loski_ProcDriver *drv);

LOSKIDRV_API int loski_closeprocesses(loski_ProcDriver *drv);

LOSKIDRV_API int loski_processerror(int error, lua_State *L);

LOSKIDRV_API void loski_proc_checkargvals(loski_ProcDriver *drv,
                                          loski_ProcArgInfo *info,
                                          loski_ProcArgFunc getarg,
                                          lua_State *L,
                                          size_t argc,
                                          size_t *size);

LOSKIDRV_API void loski_proc_toargvals(loski_ProcDriver *drv,
                                       loski_ProcArgInfo *info,
                                       loski_ProcArgFunc getarg,
                                       lua_State *L,
                                       size_t argc,
                                       void *argvals,
                                       size_t argsize);

LOSKIDRV_API void loski_proc_checkenvlist(loski_ProcDriver *drv,
                                          loski_ProcEnvInfo *info,
                                          lua_State *L,
                                          int index,
                                          size_t *size);

LOSKIDRV_API void loski_proc_toenvlist(loski_ProcDriver *drv,
                                       loski_ProcEnvInfo *info,
                                       lua_State *L,
                                       int index,
                                       void *envlist,
                                       size_t envsize);

LOSKIDRV_API int loski_createprocess(loski_ProcDriver *drv,
                                     loski_Process *proc,
                                     const char *binpath,
                                     const char *runpath,
                                     void *argvals,
                                     void *envlist,
                                     FILE *stdinput,
                                     FILE *stdoutput,
                                     FILE *stderror);

LOSKIDRV_API int loski_processstatus(loski_ProcDriver *drv,
                                     loski_Process *proc,
                                     loski_ProcStatus *status);

LOSKIDRV_API int loski_processexitval(loski_ProcDriver *drv,
                                      loski_Process *proc,
                                      int *code);

LOSKIDRV_API int loski_killprocess(loski_ProcDriver *drv,
                                   loski_Process *proc);

LOSKIDRV_API int loski_discardprocess(loski_ProcDriver *drv,
                                      loski_Process *proc);


#endif
