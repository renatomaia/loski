#include "luaosi/lproclib.h"

#include "luaosi/lauxlib.h"


LOSILIB_API void losi_defgetprocstrm (lua_State *L,
                                      const char *cls,
                                      losi_GetProcStream get)
{
	luaL_setclassdata(L, cls, "getprocstrm", get);
}

LOSILIB_API int losi_getprocstrm (lua_State *L, int idx,
                                  losi_ProcStream *stream)
{
	void *udata;
	losi_GetProcStream get = (losi_GetProcStream)
	                          luaL_getclassdata(L, idx, "getprocstrm", &udata);
	if (get) return get(udata, stream);
	return 0;
}
