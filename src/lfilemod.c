#include "luaosi/lfilelib.h"


#include "luaosi/lauxlib.h"
#include "luaosi/lproclib.h"
#include "luaosi/levtlib.h"

#include <assert.h>
#include <string.h>
#include <ctype.h>

#include <lmemlib.h>

#ifdef LOSI_DISABLE_FSDRV
#define DRVUPV	0
#define todrv(L)	NULL
#else
#define DRVUPV	1
#define todrv(L)	((losi_FSysDriver *)lua_touserdata(L, \
                                      lua_upvalueindex(DRVUPV)))
#endif

losi_FileAccessPerm permflags[3][3] = { {
	LOSI_FILEPERM_USRREAD,
	LOSI_FILEPERM_USRWRITE,
	LOSI_FILEPERM_USREXEC
},{
	LOSI_FILEPERM_GRPREAD,
	LOSI_FILEPERM_GRPWRITE,
	LOSI_FILEPERM_GRPEXEC
},{
	LOSI_FILEPERM_OTHREAD,
	LOSI_FILEPERM_OTHWRITE,
	LOSI_FILEPERM_OTHEXEC
} };

static losi_FileAccessPerm getperm (lua_State *L, int idx)
{
	losi_FileAccessPerm perm = 0;
	int i;
	for (i = 0; i<idx; ++i) {
		const char *mode = luaL_optstring(L, idx+i, "rw");
		for (; *mode; ++mode) switch (*mode) {
			case 'r': perm |= permflags[i][0]; break;
			case 'w': perm |= permflags[i][1]; break;
			case 'x': perm |= permflags[i][2]; break;
			default: luaL_error(L, "unknown perm char (got '%c')", *mode);
		}
	}
	return perm;
}

/* file = filesys.open(path [, mode [, uperm [, gperm [, operm]]]]) */
static int fs_open (lua_State *L)
{
	losi_FSysDriver *drv = todrv(L);
	losi_FileHandle *file;
	losi_FileOpenFlags flags = 0;
	losi_FileAccessPerm perm = 0;
	losi_ErrorCode err;
	const char *path = luaL_checkstring(L, 1);
	const char *mode = luaL_optstring(L, 2, "r");
	for (; *mode; ++mode) switch (*mode) {
		case 'r': flags |= LOSI_FILEOPEN_READ; break;
		case 'w': flags |= LOSI_FILEOPEN_WRITE; break;
		case 'a': flags |= LOSI_FILEOPEN_APPEND; break;
		case 't': flags |= LOSI_FILEOPEN_TRUNC; break;
		case 'n': flags |= LOSI_FILEOPEN_CREATE; break;
		case 'N': flags |= LOSI_FILEOPEN_NEW; break;
		case 'p': flags |= LOSI_FILEOPEN_PIPE; break;
		default: return luaL_error(L, "unknown mode char (got '%c')", *mode);
	}
	if (flags&(LOSI_FILEOPEN_CREATE|LOSI_FILEOPEN_NEW)) perm = getperm(L, 3);
	file = losi_newfile(L);
	err = losiF_openfile(drv, file, path, flags, perm);
	if (!err) losi_enablefile(L, -1);
	return losiL_doresults(L, 1, err);
}

/* rfile, wfile = filesys.pipe() */
static int fs_pipe (lua_State *L)
{
	losi_FSysDriver *drv = todrv(L);
	losi_FileHandle *rfile = losi_newfile(L);
	losi_FileHandle *wfile = losi_newfile(L);
	losi_ErrorCode err = losiF_pipefile(drv, rfile, wfile);
	if (!err) {
		losi_enablefile(L, -2);
		losi_enablefile(L, -1);
	}
	return losiL_doresults(L, 2, err);
}


#define tolfile(L)	((LuaFile *)luaL_checkinstance(L, 1, LOSI_FILECLS))

/* string = tostring(file) */
static int file_tostring (lua_State *L)
{
	losi_FSysDriver *drv = todrv(L);
	LuaFile *lf = tolfile(L);
	if (lf->refs == 0)
		lua_pushliteral(L, "file (closed)");
	else
		lua_pushfstring(L, "file (%d)", losiF_getfileid(drv, &lf->handle));
	return 1;
}


static int endfile (losi_FSysDriver *drv, lua_State *L, LuaFile *lf)
{
	losi_ErrorCode err = losiF_closefile(drv, &lf->handle);
	if (!err) lf->refs = 0;  /* mark file as closed */
	return losiL_doresults(L, 0, err);
}

/* getmetatable(file).__gc(file) */
static int file_gc (lua_State *L)
{
	losi_FSysDriver *drv = todrv(L);
	LuaFile *lf = tolfile(L);
	if (lf->refs != 0) endfile(drv, L, lf);
	return 0;
}


static losi_FileHandle *tofile (lua_State *L)
{
	LuaFile *lf = tolfile(L);
	if (lf->refs == 0) luaL_error(L, "attempt to use a closed file");
	return &lf->handle;
}

/* succ [, errmsg] = file:close() */
static int file_close (lua_State *L)
{
	losi_FSysDriver *drv = todrv(L);
	LuaFile *lf = tolfile(L);
	tofile(L);  /* make sure argument is an open file */
	if (lf->refs > 1) return losiL_doresults(L, 0, LOSI_ERRINUSE);
	return endfile(drv, L, lf);
}


struct FileOptionInfo {
	const char *name;
	losi_FileOption option;
};

static struct FileOptionInfo file_opts[] = {
	{"blocking", LOSI_FILEOPT_BLOCKING},
	{NULL, 0}
};

static int checkfileopt (lua_State *L,
                         int narg,
                         struct FileOptionInfo opts[])
{
	const char *name = luaL_checkstring(L, narg);
	int i;
	for (i=0; opts[i].name; i++)
		if (strcmp(opts[i].name, name) == 0)
			return opts[i].option;
	return luaL_argerror(L, narg,
	                     lua_pushfstring(L, "invalid option " LUA_QS, name));
}


/* value [, errmsg] = file:getoption(name) */
static int file_getoption (lua_State *L)
{
	losi_FSysDriver *drv = todrv(L);
	losi_FileHandle *file = tofile(L);
	losi_FileOption opt = checkfileopt(L, 2, file_opts);
	int val;
	losi_ErrorCode err = losiF_getfileopt(drv, file, opt, &val);
	if (err == 0) lua_pushboolean(L, val);
	return losiL_doresults(L, 1, err);
}


/* succ [, errmsg] = file:setoption(name, value) */
static int file_setoption (lua_State *L)
{
	losi_FSysDriver *drv = todrv(L);
	losi_FileHandle *file = tofile(L);
	losi_FileOption opt = checkfileopt(L, 2, file_opts);
	int val = lua_toboolean(L, 3);
	losi_ErrorCode err = losiF_setfileopt(drv, file, opt, val);
	return losiL_doresults(L, 0, err);
}


static size_t posrelat (ptrdiff_t pos, size_t len)
{
	if (pos >= 0) return (size_t)pos;
	else if (0u - (size_t)pos > len) return 0;
	else return len - ((size_t)-pos) + 1;
}

/* bytes [, errmsg] = file:write(data [, i [, j]]) */
static int file_write (lua_State *L)
{
	losi_FSysDriver *drv = todrv(L);
	losi_FileHandle *file = tofile(L);
	size_t sz, bytes;
	const char *data = luamem_checkstring(L, 2, &sz);
	size_t start = posrelat(luaL_optinteger(L, 3, 1), sz);
	size_t end = posrelat(luaL_optinteger(L, 4, -1), sz);
	losi_ErrorCode err;
	if (start < 1) start = 1;
	if (end > sz) end = sz;
	sz = end - start + 1;
	data += start - 1;
	err = losiF_writefile(drv, file, data, sz, &bytes);
	lua_pushinteger(L, bytes);
	return losiL_doresults(L, 1, err);
}


/* bytes [, errmsg] = file:read(memory [, i [, j]]) */
static int file_read (lua_State *L)
{
	losi_FSysDriver *drv = todrv(L);
	losi_FileHandle *file = tofile(L);
	size_t sz, bytes;
	char *buf = luamem_tomemory(L, 2, &sz);
	size_t start = posrelat(luaL_optinteger(L, 3, 1), sz);
	size_t end = posrelat(luaL_optinteger(L, 4, -1), sz);
	losi_ErrorCode err;
	if (start < 1) start = 1;
	if (end > sz) end = sz;
	sz = end - start + 1;
	buf += start - 1;
	err = losiF_readfile(drv, file, buf, sz, &bytes);
	if (!err) lua_pushinteger(L, bytes);
	return losiL_doresults(L, 1, err);
}


/*****************************************************************************
 * Library *******************************************************************
 *****************************************************************************/


static const luaL_Reg fops[] = {
	{"__tostring", file_tostring},
	{"__gc", file_gc},
	{"close", file_close},
	{"getoption", file_getoption},
	{"setoption", file_setoption},
	{"read", file_read},
	{"write", file_write},
	{NULL, NULL}
};

static const luaL_Reg lib[] = {
	{"open", fs_open},
	{"pipe", fs_pipe},
	{NULL, NULL}
};

#ifndef LOSI_DISABLE_FSDRV
static int lfreedrv (lua_State *L)
{
	losi_FSysDriver *drv = (losi_FSysDriver *)lua_touserdata(L, 1);
	losiF_freedrv(drv);
	return 0;
}
#endif

LUAMOD_API int luaopen_filesys (lua_State *L)
{
#ifndef LOSI_DISABLE_FSDRV
	/* create sentinel */
	losi_FSysDriver *drv;
	losi_ErrorCode err;
	lua_settop(L, 0);  /* dicard any arguments */
	drv = (losi_FSysDriver *)luaL_newsentinel(L, sizeof(losi_FSysDriver),
	                                             lfreedrv);
	/* initialize library */
	err = losiF_initdrv(drv);
	if (err) {
		luaL_cancelsentinel(L);
		losiL_pusherrmsg(L, err);
		return lua_error(L);
	}
#define pushsentinel(L)	lua_pushvalue(L, 1)
#else
#define pushsentinel(L)	((void)L)
#endif
	/* create abstract base file class */
	pushsentinel(L);
	luaL_newclass(L, LOSI_FILECLS, NULL, fops, DRVUPV);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	pushsentinel(L);
	luaL_setfuncs(L, lib, DRVUPV);

#ifdef LOSI_ENABLE_FILEPROCSTREAM
	losi_defgetprocstrm(L, LUA_FILEHANDLE, losiF_getfileprocstrm);
#endif
#ifdef LOSI_ENABLE_FILEEVENTS
	losi_defgetevtsrc(L, LOSI_FILECLS, losiF_getfileevtsrc);
	losi_deffreeevtsrc(L, LOSI_FILECLS, losiF_freefileevtsrc);
#endif
	return 1;
}
