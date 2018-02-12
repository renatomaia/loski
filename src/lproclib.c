#include "luaosi/lproclib.h"

#include "luaosi/lauxlib.h"


LOSKILIB_API void loski_defgetprocstrm (lua_State *L,
                                        const char *cls,
                                        loski_GetProcStream get)
{
	luaL_setclassdata(L, cls, "getprocstrm", get);
}

LOSKILIB_API int loski_getprocstrm (lua_State *L, int idx,
                                    loski_ProcStream *stream)
{
	void *udata;
	loski_GetProcStream get = (loski_GetProcStream)
	                           luaL_getclassdata(L, idx, "getprocstrm", &udata);
	if (get) return get(udata, stream);
	return 0;
}
