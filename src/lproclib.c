#include "proclib.h"
#include "loskiaux.h"

#include <assert.h>

#define LOSKI_PROCESSCLS LOSKI_PREFIX"process.ChildProcess"

#define pushresults(L,n,e) luaL_pushresults(L,n,e,loski_processerror)

static const char* getstrfield(lua_State *L, int tabidx, const char *field)
{
	const char* value;
	lua_getfield(L, tabidx, field);
	value = lua_tostring(L, -1);
	lua_pop(L, 1);
	if (value == NULL) { /* value is not a string? */
		luaL_error(L, "bad argument #%d (field "LUA_QS" must be a string)",
		              tabidx, field);
	}
	return value;
}

static const char* optstrfield(lua_State *L, int tabidx, const char *field,
                                                         const char *value)
{
	lua_getfield(L, tabidx, field);
	if (!lua_isnil(L, -1)) {
		value = lua_tostring(L, -1);
		if (value == NULL) { /* value is not a string? */
			luaL_error(L, "bad argument #%d (field "LUA_QS" must be a string)",
		                tabidx, field);
		}
	}
	lua_pop(L, 1);
	return value;
}

static FILE* optfilefield(lua_State *L, int tabidx, const char *field)
{
	lua_getfield(L, tabidx, field);
	if (!lua_isnil(L, -1)) {
		void *p = lua_touserdata(L, -1);
		if (p != NULL) {  /* value is a userdata? */
			if (lua_getmetatable(L, -1)) {  /* does it have a metatable? */
				lua_getfield(L, LUA_REGISTRYINDEX, LUA_FILEHANDLE); /* get FILE metatable */
				if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
					lua_pop(L, 3); /* remove userdata and both metatables */
					return *((FILE**)p);
				}
			}
		}
		luaL_error(L, "bad argument #%d (field "LUA_QS" must be a file)",
		              tabidx, field);
	}
	lua_pop(L, 1); /* remove userdata */
	return NULL;
}

static char ** allocargs(lua_State *L, int argc)
{
	void *userdata;
	lua_Alloc alloc = lua_getallocf(L, &userdata);
	size_t size = (argc+1)*sizeof(char *const) + sizeof(size_t);
	void *memo = alloc(userdata, NULL, 0, size);
	if (memo != NULL) {
		size_t *psize = (size_t *)memo;
		char **argv = (char **)(memo + sizeof(size_t));
		*psize = size;
		argv[argc] = NULL;
		return argv;
	}
	luaL_error(L, "not enough memory");
	return NULL; /* to avoid warnings */
}

static char *const * table2env(lua_State *L)
{
	size_t envc = 0;
	size_t size = 0;
	lua_pushnil(L);  /* first key */
	while (lua_next(L, -2) != 0) {
		if (lua_isstring(L, -2)) {
			luaL_argcheck(L, 1, !lua_isstring(L, -1),
				"field "LUA_QL("environment")" must contain only strings");
			envc++;
			size += lua_rawlen(L, -2) /* key length */
			      + lua_rawlen(L, -1) /* value length */
			      + 2;                /* '=' and '\0' */
		}
		lua_pop(L, 1);
	}
	if (envc > 0) {
		void *userdata, *memo;
		lua_Alloc alloc = lua_getallocf(L, &userdata);
		++envc;                            /* memory for the ending NULL of 'envl' */
		size += sizeof(size_t)             /* space to store the memory size */
		      + envc*(sizeof(char *const)) /* memory of 'envl' array */
		      + size*(sizeof(char));       /* memory of all 'envl[i]' strings */
		if ((memo = alloc(userdata, NULL, 0, size)) != NULL) {
			size_t *psize = (size_t *)memo;
			char **envl = (char **)(memo += sizeof(size_t));
			char *str = (char *)(memo += envc*sizeof(char *));
			*psize = size;
			envl[--envc] = NULL;
			lua_pushnil(L);  /* first key */
			while (lua_next(L, -2) != 0) {
				if (lua_isstring(L, -2)) {
					envl[--envc] = str; /* put string in 'envl' array */
					const char *c = lua_tostring(L, -2);
					while (*c) *str++ = *c++; /* copy key to string, excluding '\0' */
					*str++ = '=';
					c = lua_tostring(L, -1);
					while ((*str++ = *c++)); /* copy value to string, including '\0' */
				}
				lua_pop(L, 1); /* pop value */
			}
			return envl;
		}
		else luaL_error(L, "not enough memory");
	}
	return NULL;
}

static void freememory(lua_State *L, void *memo)
{
	if (memo != NULL) {
		void *userdata;
		lua_Alloc alloc = lua_getallocf(L, &userdata);
		memo -= sizeof(size_t);
		memo = alloc(userdata, memo, *((size_t *)memo), 0);
		assert(memo == NULL);
	}
}

static loski_Process* newproc(lua_State *L)
{
	loski_Process *proc = (loski_Process *)lua_newuserdata(L, sizeof(loski_Process));
	luaL_getmetatable(L, LOSKI_PROCESSCLS);
	lua_setmetatable(L, -2);
	return proc;
}

static int lp_create(lua_State *L)
{
	int err;
	loski_Process *proc;
	const char *exec;
	const char *path = NULL;
	char **argv = NULL;
	char *const *envl = NULL;
	FILE *stdin = NULL;
	FILE *stdout = NULL;
	FILE *stderr = NULL;
	
	if (lua_isstring(L, 1)) {
		
		int i;
		int argc = lua_gettop(L);
		argv = allocargs(L, argc); /* TODO: memory leak in case of arg erros */
		argv[0] = (char *)luaL_checkstring(L, 1);
		for (i = 1; i < argc; ++i)
			argv[i] = (char *)luaL_checkstring(L, i+1);
		exec = argv[0];
		
	} else if (lua_istable(L, 1)) {
		
		lua_settop(L, 1);
		exec = getstrfield(L, 1, "execfile");
		path = optstrfield(L, 1, "runpath", path);
		stdin = optfilefield(L, 1, "stdin");
		stdout = optfilefield(L, 1, "stdout");
		stderr = optfilefield(L, 1, "stderr");
		
		lua_getfield(L, 1, "arguments");
		if (lua_istable(L, 2)) {
			size_t i;
			size_t argc = lua_rawlen(L, 2);
			argv = allocargs(L, argc+1); /* TODO: memory leak in case of arg erros */
			argv[0] = (char *)exec;
			for(i = 1; i <= argc; ++i) {
				lua_rawgeti(L, 2, i);
				luaL_argcheck(L, 1, !lua_isstring(L, 3),
					"field "LUA_QL("arguments")" must contain only strings");
				argv[i] = (char *)lua_tostring(L, 3);
				lua_pop(L, 1); /* pop an argument string */
			}
		} else if (!lua_isnil(L, 2)) {
			luaL_argerror(L, 1, "field "LUA_QL("arguments")" must be a table");
		}
		lua_pop(L, 1); /* pop field 'arguments' */
		
		lua_getfield(L, 1, "environment");
		if (lua_istable(L, 2)) {
			envl = table2env(L);
		} else if (!lua_isnil(L, 2)) {
			luaL_argerror(L, 1, "field "LUA_QL("environment")" must be a table");
		}
		lua_pop(L, 1); /* pop field 'environment' */
		
	} else {
		return luaL_argerror(L, 1, "table or string expected");
	}
	proc = newproc(L); /* push a new proc structure on the stack */
	err = loski_createprocess(proc, exec, path, argv, envl, stdin, stdout, stderr);
	freememory(L, (void *)argv);
	freememory(L, (void *)envl);
	return pushresults(L, 1, err); /* return process */
}

const char *StatusName[] = { "running", "suspended", "dead" };

static int lp_status(lua_State *L)
{
	loski_Process* proc = (loski_Process *)luaL_checkudata(L, 1, LOSKI_PROCESSCLS);
	loski_ProcStatus status;
	int err = loski_processstatus(proc, &status);
	if (err == 0) lua_pushstring(L, StatusName[status]);
	return pushresults(L, 1, err);
}

static int lp_exitval(lua_State *L)
{
	loski_Process* proc = (loski_Process *)luaL_checkudata(L, 1, LOSKI_PROCESSCLS);
	int code;
	int err = loski_processexitval(proc, &code);
	if (err) lua_pushnil(L);
	else lua_pushinteger(L, code);
	return 1;
}

static int lp_kill(lua_State *L)
{
	loski_Process* proc = (loski_Process *)luaL_checkudata(L, 1, LOSKI_PROCESSCLS);
	int err = loski_killprocess(proc);
	return pushresults(L, 0, err);
}

static int lp_gc(lua_State *L)
{
	loski_Process* proc = (loski_Process *)luaL_checkudata(L, 1, LOSKI_PROCESSCLS);
	loski_discardprocess(proc);
	return 0;
}

static int lp_sentinel(lua_State *L)
{
	loski_closeprocesses();
	return 0;
}

static const luaL_Reg lib[] = {
	{"create", lp_create},
	{NULL, NULL}
};

static const luaL_Reg cls[] = {
	{"status", lp_status},
	{"exitval", lp_exitval},
	{"kill", lp_kill},
	{"__gc", lp_gc},
	{NULL, NULL}
};

LUAMOD_API int luaopen_process(lua_State *L)
{
	/* create sentinel */
	luaL_newsentinel(L, lp_sentinel);
	/* create process class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newclass(L, LOSKI_PROCESSCLS, cls, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	lua_pushvalue(L, -2);  /* push sentinel */
	luaL_setfuncs(L, lib, 1);
	/* initialize library */
	if (loski_openprocesses() != 0) {
		luaL_cancelsentinel(L);
		return luaL_error(L, "unable to initialize library");
	}
	return 1;
}
