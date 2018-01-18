#ifndef lprocaux_h
#define lprocaux_h


#include "loskiconf.h"
#include "loskiaux.h"

#include <proclib.h>

#include <lua.h>

#define LOSKI_PROCESSCLS LOSKI_PREFIX"ChildProcess"

LOSKILIB_API int loski_isprocess (lua_State *L, int idx);
LOSKILIB_API loski_Process *loski_toprocess (lua_State *L, int idx);


#define LOSKI_PROCSTREAMCONV LOSKI_PREFIX"ToProcessStreamOperation"

typedef int (*loski_ProcStreamConv)(lua_State *L, int idx, 
                                    loski_ProcStream *stream);

#define loski_setprocstreamconv(L, cls, func) \
	loskiU_setclassop(L, LOSKI_PROCSTREAMCONV, cls, func)

#define loski_getprocstreamconv(L) \
	loskiU_getvalueop(L, LOSKI_PROCSTREAMCONV)


#endif
