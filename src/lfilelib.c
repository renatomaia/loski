#include "luaosi/lfilelib.h"


#define tolfile(L,i)	((LuaFile *)luaL_testudata(L, i, LOSI_FILECLS))

LOSILIB_API losi_FileHandle *losi_newfile (lua_State *L)
{
	LuaFile *lf = (LuaFile *)lua_newuserdata(L, sizeof(LuaFile));
	lf->refs = 0;
	luaL_setmetatable(L, LOSI_FILECLS);
	return &lf->handle;
}

LOSILIB_API void losi_enablefile (lua_State *L, int idx)
{
	LuaFile *lf = tolfile(L, idx);
	if (lf) ++(lf->refs);
}

LOSILIB_API losi_FileHandle *losi_tofile (lua_State *L, int idx)
{
	LuaFile *lf = tolfile(L, idx);
	if (!lf || lf->refs == 0) return NULL;
	return &lf->handle;
}
