#ifndef luaosi_lproclib_h
#define luaosi_lproclib_h


#include "luaosi/proclib.h"


#define LOSI_PROCESSCLS LOSI_PREFIX"Process"

#define losi_toprocess(L,i)	((losi_Process *) \
                               luaL_testudata(L, i, LOSI_PROCESSCLS))

#define losi_isprocess(L,i)	(losi_toprocess(L, i) != NULL)


typedef int (*losi_GetProcStream)(void *udata, losi_ProcStream *stream);

LOSILIB_API void losi_defgetprocstrm (lua_State *L,
                                      const char *cls,
                                      losi_GetProcStream get);
LOSILIB_API int losi_getprocstrm (lua_State *L, int idx,
                                  losi_ProcStream *stream);


#endif
