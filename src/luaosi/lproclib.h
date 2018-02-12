#ifndef luaosi_lproclib_h
#define luaosi_lproclib_h


#include "luaosi/proclib.h"


#define LOSKI_PROCESSCLS LOSKI_PREFIX"Process"

#define loski_toprocess(L,i)	((loski_Process *) \
                               luaL_testudata(L, i, LOSKI_PROCESSCLS))

#define loski_isprocess(L,i)	(loski_toprocess(L, i) != NULL)


typedef int (*loski_GetProcStream)(void *udata, loski_ProcStream *stream);

LOSKILIB_API void loski_defgetprocstrm (lua_State *L,
                                        const char *cls,
                                        loski_GetProcStream get);
LOSKILIB_API int loski_getprocstrm (lua_State *L, int idx,
                                    loski_ProcStream *stream);


#endif
