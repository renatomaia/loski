#include "proclib.h"
#include "proctab.h"

/* See: http://www.steve.org.uk/Reference/Unix/faq_2.html */

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <lauxlib.h>

#define TABLE_INITSZ 4
#define ERRNO_NO_EXIT_VALUE (-1)

static char hastable = 0;
static loski_ProcTable table;


static int stdfile(int stdfd, FILE* file)
{
	int fd = fileno(file);
	if (fd != stdfd) {
		int res = dup2(fd, stdfd);
		if (res != fd) return -1;
		if (fd > 2 && fclose(file) == EOF) return -1;
	}
	return 0;
}

static int execvep(const char *file, char *const argv[], char *const envp[])
{
	if (!strchr(file, '/')) {
		const char *prefix = getenv("PATH");
		
		size_t namelen = strlen(file);
		while(prefix && *prefix != '\0')
		{
			char path[PATH_MAX];
			const char* pfxend = strchr(prefix, ':');
			size_t pfxlen = pfxend ? (pfxend-prefix) : strlen(prefix);
			
			if (pfxlen+1+namelen >= PATH_MAX) {
				errno = ENAMETOOLONG;
				return -1;
			}
			
			strncpy(path, prefix, pfxlen);
			if (pfxlen > 0) path[pfxlen++] = '/';
			path[pfxlen] = '\0';
			strcat(path, file);
			
			execve(path, argv, envp);
			
			if (errno != ENOENT) return -1;
			
			prefix = pfxend ? (pfxend+1) : NULL;
		}
	}
	return execve(file, argv, envp);
}

static void childsignal(int signo)
{
	pid_t pid;
	int status;
	signal(SIGCHLD, childsignal); /* to catch SIGCHLD for next time */
	pid = wait(&status); /* After wait, child is definitely freed */
	if (hastable) {
		loski_Process *proc = loski_proctabdel(&table, pid);
		if (proc) {
			proc->pid = 0;
			proc->status = status;
		}
	}
}

LOSKIDRV_API void loski_proc_checkargvals(lua_State *L,
                                          loski_ProcArgFunc getarg,
                                          size_t argc,
                                          size_t *size,
                                          loski_ProcArgInfo *info)
{
	int i;
	for (i = 0; i < argc; ++i) getarg(L, i); /* 'getarg' checks args */
	*size = (argc+1)*sizeof(const char *); /* args + NULL */
}

LOSKIDRV_API void loski_proc_toargvals(lua_State *L,
                                       loski_ProcArgFunc getarg,
                                       size_t argc,
                                       void *argvals,
                                       size_t argsize,
                                       loski_ProcArgInfo *info)
{
	const char **argv = (const char **)argvals;
	int i;
	for (i = 0; i < argc; ++i) argv[i] = getarg(L, i);
	argv[argc] = NULL;
}

LOSKIDRV_API void loski_proc_checkenvlist(lua_State *L,
                                          int index,
                                          size_t *size,
                                          loski_ProcEnvInfo *count)
{
	size_t chars = 0;
	*count = 0;
	lua_pushnil(L);  /* first key */
	while (lua_next(L, index) != 0) {
		if (lua_isstring(L, -2)) {
			const char *name = lua_tostring(L, -2);
			while (*name) {
				if (*name == '=') luaL_argerror(L, 1,
					"environment variable names containing '=' are not allowed");
				chars++;
				name++;
			}
			luaL_argcheck(L, 1, !lua_isstring(L, -1),
				"value of environment variables must be strings");
			chars += strlen(lua_tostring(L, -1));
			(*count)++;
		}
		lua_pop(L, 1);
	}
	*size = ((*count)+1)*(sizeof(char *)) /* the 'envl' array + NULL */
	      + (chars+2*(*count))*(sizeof(char)); /* #key + #value + '=' + '\0' */
}

LOSKIDRV_API void loski_proc_toenvlist(lua_State *L,
                                       int index,
                                       void *envlist,
                                       size_t envsize,
                                       loski_ProcEnvInfo *count)
{
	char **envl = (char **)envlist;
	char *str = (char *)(envlist + ((*count)+1)*sizeof(char *));
	int i = 0;
	lua_pushnil(L);  /* first key */
	while (lua_next(L, index) != 0) {
		if (lua_isstring(L, -2)) {
			const char *c = lua_tostring(L, -2);
			envl[i++] = str; /* put string in 'envl' array */
			while (*c) *str++ = *c++; /* copy key to string, excluding '\0' */
			*str++ = '=';
			c = lua_tostring(L, -1);
			while ((*str++ = *c++)); /* copy value to string, including '\0' */
		}
		lua_pop(L, 1); /* pop value */
	}
	envl[i] = NULL; /* put NULL to mark the end of 'envl' array */
}

/* TODO: let application provide a memory allocation function */
LOSKIDRV_API int loski_openprocesses()
{
	if (!hastable) {
		signal(SIGCHLD, childsignal);
		if (loski_proctabinit(&table, TABLE_INITSZ) == 0) {
			hastable = 1;
			return 0;
		}
	}
	return -1;
}

LOSKIDRV_API int loski_closeprocesses()
{
	if (hastable) {
		signal(SIGCHLD, SIG_DFL);
		loski_proctabclose(&table);
		hastable = 0;
		return 0;
	}
	return -1;
}

LOSKIDRV_API int loski_processerror(int error, lua_State *L)
{
	switch (error) {
		case ERRNO_NO_EXIT_VALUE: lua_pushliteral(L, "process did not exit"); break;
		default: lua_pushstring(L, strerror(error)); break;
	}
	return 0;
}

LOSKIDRV_API int loski_createprocess(loski_Process *proc,
                                     const char *binpath,
                                     const char *runpath,
                                     void *argvals,
                                     void *envlist,
                                     FILE *stdin,
                                     FILE *stdout,
                                     FILE *stderr)
{
	proc->pid = fork();
	proc->status = 0;
	if (proc->pid == -1) return errno;
	else if (proc->pid > 0) {
		// TODO: http://www.cs.utah.edu/dept/old/texinfo/glibc-manual-0.02/library_21.html#SEC368
		loski_proctabput(&table, proc);
		return 0;
	}
	/* child process */
	int res = 0;
	if (res == 0 && stdin  ) res = stdfile(0, stdin);
	if (res == 0 && stdout ) res = stdfile(1, stdout);
	if (res == 0 && stderr ) res = stdfile(2, stderr);
	if (res == 0 && runpath) res = chdir(runpath);
	if (res == 0) {
		int max = sysconf(_SC_OPEN_MAX);
		if (max > 0) {
			int i;
			for (i=3; i<max; ++i) close(i); /* close all open file descriptors */
			if (envlist) execvep(binpath, (char *const *)argvals, (char *const *)envlist);
			else execvp(binpath, (char *const *)argvals);
		}
	}
	_exit(errno);
}

LOSKIDRV_API int loski_processstatus(loski_Process *proc,
                                     loski_ProcStatus *status)
{
	if (proc->pid != 0) {
		pid_t res = waitpid(proc->pid, &proc->status, WNOHANG);
		if (res == -1) return errno;
		
		if (proc->status == 0) {
			if (res == proc->pid) {
				*status = LOSKI_DEADPROC;
				proc->pid = 0;
			} else {
				*status = LOSKI_RUNNINGPROC;
			}
		} else if (WIFEXITED(proc->status)) {
			*status = LOSKI_DEADPROC;
			proc->pid = 0;
		} else if (WIFSIGNALED(proc->status)) {
			*status = LOSKI_DEADPROC;
			proc->pid = 0;
		} else if (WIFSTOPPED(proc->status)) {
			*status = LOSKI_SUSPENDEDPROC;
		} else if (WIFCONTINUED(proc->status)) {
			*status = LOSKI_RUNNINGPROC;
		}
	}
	else *status = LOSKI_DEADPROC;
	return 0;
}

LOSKIDRV_API int loski_processexitval(loski_Process *proc, int *code)
{
	if ( (proc->pid == 0) && WIFEXITED(proc->status) ) {
		*code = WEXITSTATUS(proc->status);
		return 0;
	}
	return ERRNO_NO_EXIT_VALUE;
}

LOSKIDRV_API int loski_killprocess(loski_Process *proc)
{
	if (proc->pid != 0) {
		int res = kill(proc->pid, SIGKILL);
		if (res == -1) return errno;
	}
	return 0;
}

LOSKIDRV_API int loski_discardprocess(loski_Process *proc)
{
	if (proc->pid>0 && loski_proctabdel(&table, proc->pid)) {
		proc->pid = -1;
		proc->status = 0;
		return 0;
	}
	return -1;
}
