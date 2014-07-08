#ifndef proclibapi_h
#define proclibapi_h

#include "loskiconf.h"

#include <stdio.h> /* definition of 'FILE*' */
#include <lua.h> /* to copy error messages to Lua */

typedef struct loski_Process loski_Process;

typedef const char * (*loski_ProcArgFunc) (lua_State *L, size_t index);

enum loski_ProcStatus { LOSKI_RUNNINGPROC, LOSKI_SUSPENDEDPROC, LOSKI_DEADPROC };
typedef enum loski_ProcStatus loski_ProcStatus;

LOSKIDRV_API int loski_openprocesses();

LOSKIDRV_API int loski_closeprocesses();

LOSKIDRV_API int loski_processerror(int error, lua_State *L);

LOSKIDRV_API void loski_proc_checkargvals(lua_State *L,
                                          loski_ProcArgFunc getarg,
                                          size_t argc,
                                          size_t *size,
                                          loski_ProcArgInfo *info);

LOSKIDRV_API void loski_proc_toargvals(lua_State *L,
                                       loski_ProcArgFunc getarg,
                                       size_t argc,
                                       void *argvals,
                                       size_t argsize,
                                       loski_ProcArgInfo *info);

LOSKIDRV_API void loski_proc_checkenvlist(lua_State *L,
                                          int index,
                                          size_t *size,
                                          loski_ProcEnvInfo *info);

LOSKIDRV_API void loski_proc_toenvlist(lua_State *L,
                                       int index,
                                       void *envlist,
                                       size_t envsize,
                                       loski_ProcEnvInfo *info);

LOSKIDRV_API int loski_createprocess(loski_Process *proc,
                                     const char *binpath,
                                     const char *runpath,
                                     void *argvals,
                                     void *envlist,
                                     FILE *stdinput,
                                     FILE *stdoutput,
                                     FILE *stderror);

LOSKIDRV_API int loski_processstatus(loski_Process *proc,
                                     loski_ProcStatus *status);

LOSKIDRV_API int loski_processexitval(loski_Process *proc, int *code);

LOSKIDRV_API int loski_killprocess(loski_Process *proc);

LOSKIDRV_API int loski_discardprocess(loski_Process *proc);


#endif
