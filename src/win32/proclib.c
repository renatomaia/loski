#include "proclib.h"

#include <assert.h>
#include <io.h> /* _get_osfhandle */
#include <stdlib.h> /* qsort */
#include <string.h> /* strcmp */
#include <lauxlib.h>

#define MAX_CMDLINE 32768
#define PROCESS_KILL_EXITCODE 0xFFFFFFFF
#define ERRNO_UNABLE_TO_GET_STDSTRM 0x20000000 /* no system error code has bit 29 set */
#define ERRNO_PROCESS_STILL_ACTIVE 0x20000001 /* no system error code has bit 29 set */
#define ERRNO_TOO_MANY_ARGUMENTS 0x20000002 /* no system error code has bit 29 set */

static int pstrcmp(const void *p1, const void *p2) /* compare strings through pointers */
{
	return strcmp(*(const char **)p1, *(const char **)p2);
}

static int makeCommandLine(char *const argvals[], LPTSTR cmdline)
{
	int c = 0;
	int i;
	for (i = 0; argvals[i]; ++i)
	{
		int s = 1;
		LPCTSTR p = argvals[i];
		if (c+3 > MAX_CMDLINE) return 0;
		cmdline[c++] = TEXT('"');
		for (p = argvals[i]; *p; ++p)
		{
			switch (*p) {
			case TEXT('"'):
				if (c+s+3 > MAX_CMDLINE) return 0;
				while (s--) cmdline[c++] = TEXT('\\');
				break;
			case TEXT('\\'):
				++s;
				break;
			default:
				s = 1;
				break;
			}
			if (c+3 > MAX_CMDLINE) return 0;
			cmdline[c++] = *p;
		}
		cmdline[c++] = TEXT('"');
		cmdline[c++] = TEXT(' ');
	}
	cmdline[c++] = TEXT('\0');
	return 1;
}

static DWORD createInheriableHandle(HANDLE cph, HANDLE src, LPHANDLE dst)
{
	return !DuplicateHandle(cph, /* source process */
	                        src, /* source handle */
	                        cph, /* destiny process */
	                        dst, /* duplicate handle */
	                        0x0, /* access: ignored due to last param */
	                        TRUE, /* create an inheritable handle? */
	                        DUPLICATE_SAME_ACCESS)
		? GetLastError()
		: 0;
}

static DWORD resolveStdStream(FILE* provided, DWORD standard, LPHANDLE handle)
{
	if (provided) {
		*handle = (HANDLE)_get_osfhandle(_fileno(provided));
	} else {
		*handle = GetStdHandle(standard);
		if (*handle == INVALID_HANDLE_VALUE) return GetLastError();
		if (*handle == NULL) return ERRNO_UNABLE_TO_GET_STDSTRM;
	}
	return 0;
}

LOSIDRV_API void losi_proc_checkargvals(losi_ProcDriver *drv,
                                          losi_ProcArgInfo *info,
                                          losi_ProcArgFunc getarg,
                                          lua_State *L,
                                          size_t argc,
                                          size_t *size)
{
	size_t c = 0;
	size_t i;
	for (i = 0; i < argc; ++i) /* 'getarg' checks args */
	{
		LPCTSTR p = getarg(L, i); /* TODO: convert to ASCII or Unicode? */
		size_t bs = 1; /* backslashes that shall precede a '"' */
		if (c+3 > MAX_CMDLINE) luaL_argerror(L, 1, "arguments too long");
		while (*p)
		{
			switch (*p)
			{
			case TEXT('\\'):
				++bs;
				break;
			case TEXT('"'):
				c += bs; /* additional preceding '\\' */
			default:
				bs = 1;
				break;
			}
			if (c+3 > MAX_CMDLINE) luaL_argerror(L, 1, "arguments too long");
			++c; /* original char */
			++p;
		}
		c += 3; /* opening and closing '"' and ' ' */
	}
	*size = (c+1)*sizeof(TCHAR); /* cmdline + '\0' */
}

LOSIDRV_API void losi_proc_toargvals(losi_ProcDriver *drv,
                                       losi_ProcArgInfo *info,
                                       losi_ProcArgFunc getarg,
                                       lua_State *L,
                                       size_t argc,
                                       void *argvals,
                                       size_t argsize)
{
	LPTSTR cmdline = (LPTSTR)argvals;
	size_t c = 0; /* characers written */
	size_t i;
	for (i = 0; i < argc; ++i)
	{
		LPCTSTR p;
		size_t bs;
		p = getarg(L, i); /* TODO: convert to ASCII or Unicode? */
		bs = 1; /* backslashes that shall precede a '"' */
		assert(c+3 > MAX_CMDLINE);
		cmdline[c++] = TEXT('"'); /* opening '"' */
		while (*p)
		{
			switch (*p)
			{
			case TEXT('\\'):
				++bs;
				break;
			case TEXT('"'):
				while (bs--) cmdline[c++] = TEXT('\\');
			default:
				bs = 1;
				break;
			}
			assert(c+3 > MAX_CMDLINE);
			cmdline[c++] = *p++;
		}
		cmdline[c++] = TEXT('"'); /* closing '"' */
		cmdline[c++] = TEXT(' ');
	}
	cmdline[c++] = TEXT('\0');
}

LOSIDRV_API void losi_proc_checkenvlist(losi_ProcDriver *drv,
                                          losi_ProcEnvInfo *count,
                                          lua_State *L,
                                          int index,
                                          size_t *size)
{
	size_t chars = 0;
	*count = 0;
	lua_pushnil(L);  /* first key */
	while (lua_next(L, index) != 0) {
		if (lua_isstring(L, -2)) {
			LPCTSTR name = lua_tostring(L, -2); /* TODO: convert to ASCII or Unicode? */
			while (*name) {
				if (*name == TEXT('=')) luaL_argerror(L, 1,
					"environment variable names containing '=' are not allowed");
				chars++;
				name++;
			}
			luaL_argcheck(L, 1, !lua_isstring(L, -1),
				"value of environment variables must be strings");
			chars += strlen(lua_tostring(L, -1)); /* TODO: convert to ASCII or Unicode? */
			(*count)++;
		}
		lua_pop(L, 1);
	}
	*size = (chars+2*(*count)+1)*(sizeof(TCHAR))/*#key+#val + 2*('='+'\0') + '\0'*/
	      + ((*count)*sizeof(LPCTSTR)); /* env. var. names to be sorted */
}

LOSIDRV_API void losi_proc_toenvlist(losi_ProcDriver *drv,
                                       losi_ProcEnvInfo *count,
                                       lua_State *L,
                                       int index,
                                       void *envlist,
                                       size_t envsize)
{
	LPTSTR str = (LPTSTR)envlist;
	LPCTSTR *keys = (LPCTSTR *)((char *)envlist + envsize - (*count)*sizeof(LPCTSTR));
	size_t i = 0;
	lua_pushnil(L);  /* first key */
	while (lua_next(L, index) != 0) {
		if (lua_isstring(L, -2)) keys[i++] = lua_tostring(L, -2); /* TODO: convert to ASCII or Unicode? */
		lua_pop(L, 1); /* pop value */
	}
	qsort((void *)keys, *count, sizeof(LPCTSTR), pstrcmp);
	for (i = 0; i < *count; ++i)
	{
		LPCTSTR c = keys[i];
		while (*c) *str++ = *c++; /* copy key to string, excluding '\0' */
		*str++ = '=';
		lua_getfield(L, index, keys[i]);
		c = lua_tostring(L, -1); /* TODO: convert to ASCII or Unicode? */
		while ((*str++ = *c++)); /* copy value to string, including '\0' */
		lua_pop(L, 1); /* pop value */
	}
	*str = '\0'; /* mark of the end of string block */
}

LOSIDRV_API int losi_openprocesses(losi_ProcDriver *drv)
{
	return 0;
}

LOSIDRV_API int losi_closeprocesses(losi_ProcDriver *drv)
{
	return 0;
}

LOSIDRV_API int losi_processerror(int error, lua_State *L)
{
	switch (error) {
		case ERRNO_UNABLE_TO_GET_STDSTRM:
			lua_pushliteral(L, "unable to get handle to standard stream");
			break;
		case ERRNO_PROCESS_STILL_ACTIVE:
			lua_pushliteral(L, "process did not exit");
			break;
		default: {
			LPVOID lpMsgBuf;
			DWORD chars = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			                            FORMAT_MESSAGE_FROM_SYSTEM |
			                            FORMAT_MESSAGE_IGNORE_INSERTS,
			                            NULL, /* ignored message source info (from system) */
			                            error,
			                            0, /* message language: use default options */
			                            (LPTSTR) &lpMsgBuf,
			                            0, /* buffer size: no min. size to be allocated */
			                            NULL); /* ignored message insert values */
			if (chars == 0) return GetLastError();
			lua_pushlstring(L, (const char *)lpMsgBuf, chars*sizeof(TCHAR));
			LocalFree(lpMsgBuf);
		} break;
	}
	return 0;
}

LOSIDRV_API int losi_createprocess(losi_ProcDriver *drv,
                                     losi_Process *proc,
                                     const char *binpath,
                                     const char *runpath,
                                     void *argvals,
                                     void *envlist,
                                     FILE *stdinput,
                                     FILE *stdoutput,
                                     FILE *stderror)
{
	DWORD res = 0;
	BOOL created;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;
	if (stdinput || stdoutput || stderror) {
		HANDLE instream;
		HANDLE outstream;
		HANDLE errstream;
		HANDLE currProc = GetCurrentProcess();
		si.dwFlags |= STARTF_USESTDHANDLES;
		res = resolveStdStream(stdinput, STD_INPUT_HANDLE, &instream);
		if (res) return res;
		res = resolveStdStream(stdoutput, STD_OUTPUT_HANDLE, &outstream);
		if (res) return res;
		res = resolveStdStream(stderror, STD_ERROR_HANDLE, &errstream);
		if (res) return res;
		res = createInheriableHandle(currProc, instream, &(si.hStdInput));
		if (res) return res;
		res = createInheriableHandle(currProc, outstream, &(si.hStdOutput));
		if (res) {
			CloseHandle(si.hStdInput);
			return res;
		}
		res = createInheriableHandle(currProc, errstream, &(si.hStdError));
		if (res) {
			CloseHandle(si.hStdInput);
			CloseHandle(si.hStdOutput);
			return res;
		}
	}
	created = CreateProcess(binpath,
	                        (LPTSTR)argvals,
	                        NULL, /* do not inherit new process handle */
	                        NULL, /* do not inherit new process' thread handle */
	                        si.dwFlags & STARTF_USESTDHANDLES, /* inherit handles? */
	                        NORMAL_PRIORITY_CLASS, /* process priority */
	                        (LPVOID)envlist,
	                        runpath,
	                        &si,
	                        &proc->pi);
	if (!created) res = GetLastError();
	if (si.dwFlags & STARTF_USESTDHANDLES) {
		CloseHandle(si.hStdInput);
		CloseHandle(si.hStdOutput);
		CloseHandle(si.hStdError);
	}
	if (!created) return res;
	CloseHandle(proc->pi.hThread);
	proc->exitcode = STILL_ACTIVE;
	return 0;
}

LOSIDRV_API int losi_processstatus(losi_ProcDriver *drv,
                                     losi_Process *proc,
                                     losi_ProcStatus *status)
{
	if (proc->exitcode == STILL_ACTIVE) {
		int dummy;
		DWORD error = losi_processexitval(drv, proc, &dummy);
		if (error == ERRNO_PROCESS_STILL_ACTIVE) {
			*status = LOSI_RUNNINGPROC;
			return 0;
		} else if (error) {
			return error;
		}
	}
	*status = LOSI_DEADPROC;
	return 0;
}

LOSIDRV_API int losi_processexitval(losi_ProcDriver *drv,
                                      losi_Process *proc,
                                      int *code)
{
	if (proc->exitcode == STILL_ACTIVE) {
		BOOL success = GetExitCodeProcess(proc->pi.hProcess, &proc->exitcode);
		if (!success) {
			return GetLastError();
		} else if (proc->exitcode == STILL_ACTIVE) {
			return ERRNO_PROCESS_STILL_ACTIVE;
		}
	}
	*code = (int)proc->exitcode;
	return 0;
}

LOSIDRV_API int losi_killprocess(losi_ProcDriver *drv,
                                   losi_Process *proc)
{
	BOOL success = TerminateProcess(proc->pi.hProcess, PROCESS_KILL_EXITCODE);
	if (!success) return GetLastError();
	return 0;
}

LOSIDRV_API int losi_discardprocess(losi_ProcDriver *drv,
                                      losi_Process *proc)
{
	CloseHandle(proc->pi.hProcess);
	return 0;
}
