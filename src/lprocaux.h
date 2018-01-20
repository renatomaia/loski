#ifndef lprocaux_h
#define lprocaux_h


#include "proclib.h"
#include "loskiaux.h"


#define LOSKI_PROCESSCLS LOSKI_PREFIX"ChildProcess"

#define loski_toprocess(L,i)	((loski_Process *) \
                               luaL_testudata(L, i, LOSKI_PROCESSCLS))

#define loski_isprocess(L,i)	(loski_toprocess(L, i) != NULL)


#define LOSKI_PROCSTREAMCONV LOSKI_PREFIX"ToProcessStreamOperation"

typedef int (*loski_ProcStreamConv)(lua_State *L, int idx, 
                                    loski_ProcStream *stream);

#define loski_setprocstreamconv(L, cls, func) \
	loskiL_setclassop(L, LOSKI_PROCSTREAMCONV, cls, func)

#define loski_getprocstreamconv(L) \
	loskiL_getvalueop(L, LOSKI_PROCSTREAMCONV)


#endif
