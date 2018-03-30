#ifndef luaosi_lfilelib_h
#define luaosi_lfilelib_h


#include "luaosi/filelib.h"
#include "luaosi/lauxlib.h"


#define LOSI_FILECLS LOSI_PREFIX"RegularFile"

typedef struct LuaFile {
	losi_FileHandle handle;
	size_t refs;
} LuaFile;

LOSILIB_API losi_FileHandle *losi_newfile (lua_State *L);

LOSILIB_API void losi_enablefile (lua_State *L, int idx);

LOSILIB_API losi_FileHandle *losi_tofile (lua_State *L, int idx);

#define losi_isfile(L,i)	(losi_tofile(L, i) != NULL)


#endif
