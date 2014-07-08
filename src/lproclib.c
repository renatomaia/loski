#include "proclib.h"
#include "loskiaux.h"

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

static const char *argfromindex (lua_State *L, size_t index)
{
	return luaL_checkstring(L, index+1);
}

static const char *argfromtable (lua_State *L, size_t index)
{
	const char *argv;
	if (index == 0) {
		argv = getstrfield(L, 1, "execfile");
	} else {
		lua_rawgeti(L, 2, index);
		luaL_argcheck(L, 1, !lua_isstring(L, 3),
			"field "LUA_QL("arguments")" must contain only strings");
		argv = (char *)lua_tostring(L, 3);
		lua_pop(L, 1); /* pop an argument string */
	}
	return argv;
}




typedef struct LuaProcess {
	loski_Process process;
	int closed;
} LuaProcess;


#define tolproc(L)	((LuaProcess *)luaL_checkinstance(L, 1, LOSKI_PROCESSCLS))


static int lp_create(lua_State *L)
{
	int err;
	loski_Process *proc;
	const char *exec;
	const char *path = NULL;
	FILE *stdinput = NULL;
	FILE *stdoutput = NULL;
	FILE *stderror = NULL;
	void *argv = NULL;
	void *envl = NULL;
	size_t argsz = 0;
	size_t envsz = 0;
	loski_ProcArgInfo arginf;
	loski_ProcEnvInfo envinf;
	
	if (lua_isstring(L, 1)) {

		size_t argc = lua_gettop(L);
		exec = luaL_checkstring(L, 1);
		loski_proc_checkargvals(L, argfromindex, argc, &argsz, &arginf);
		argv = luaL_alloctemporary(L, argsz);
		loski_proc_toargvals(L, argfromindex, argc, argv, argsz, &arginf);

	} else if (lua_istable(L, 1)) {
		size_t argc = 1; /* execfile */

		lua_settop(L, 1);
		exec = getstrfield(L, 1, "execfile");
		path = optstrfield(L, 1, "runpath", path);
		stdinput = optfilefield(L, 1, "stdin");
		stdoutput = optfilefield(L, 1, "stdout");
		stderror = optfilefield(L, 1, "stderr");

		lua_getfield(L, 1, "arguments");
		if (lua_istable(L, 2)) {
			argc += lua_rawlen(L, 2);
			loski_proc_checkargvals(L, argfromtable, argc, &argsz, &arginf);
		} else if (!lua_isnil(L, 2)) {
			luaL_argerror(L, 1, "field "LUA_QL("arguments")" must be a table");
		}

		lua_getfield(L, 1, "environment");
		if (lua_istable(L, -1)) {
			size_t index = lua_gettop(L);
			loski_proc_checkenvlist(L, index, &envsz, &envinf);
			envl = luaL_alloctemporary(L, envsz);
			loski_proc_toenvlist(L, index, envl, envsz, &envinf);
		} else if (!lua_isnil(L, -1)) {
			luaL_argerror(L, 1, "field "LUA_QL("environment")" must be a table");
		}
		lua_pop(L, 1); /* pop field 'environment' */

		if (lua_istable(L, 2)) {
			argv = luaL_alloctemporary(L, argsz);
			loski_proc_toargvals(L, argfromtable, argc, argv, argsz, &arginf);
		}
		lua_pop(L, 1); /* pop field 'arguments' */

	} else {
		return luaL_argerror(L, 1, "table or string expected");
	}
	/* push a new proc structure on the stack */
	proc = (loski_Process *)lua_newuserdata(L, sizeof(loski_Process));
	err = loski_createprocess(proc, exec, path, argv, envl, stdinput, stdoutput, stderror);
	if (argv != NULL) luaL_freetemporary(L, argv, argsz);
	if (envl != NULL) luaL_freetemporary(L, envl, envsz);
	if (err) return pushresults(L, 1, err); /* return process */
	/* setup the new proc structure */
	luaL_getmetatable(L, LOSKI_PROCESSCLS);
	lua_setmetatable(L, -2);
	return 1;
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
	if (err == 0) lua_pushinteger(L, code);
	return pushresults(L, 1, err);
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

static int lp_tostring (lua_State *L)
{
	loski_Process *proc = (loski_Process *)luaL_checkudata(L, 1, LOSKI_PROCESSCLS);
	lua_pushfstring(L, "process (%p)", proc);
	return 1;
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
	{"__tostring", lp_tostring},
	{NULL, NULL}
};

LUAMOD_API int luaopen_process(lua_State *L)
{
	/* create sentinel */
	luaL_newsentinel(L, lp_sentinel);
	/* initialize library */
	if (loski_openprocesses() != 0) {
		luaL_cancelsentinel(L);
		return luaL_error(L, "unable to initialize library");
	}
	/* create process class */
	lua_pushvalue(L, -1);  /* push sentinel */
	luaL_newclass(L, LOSKI_PROCESSCLS, cls, 1);
	lua_pop(L, 1);  /* remove new class */
	/* create library table */
	luaL_newlibtable(L, lib);
	lua_pushvalue(L, -2);  /* push sentinel */
	luaL_setfuncs(L, lib, 1);
	return 1;
}
