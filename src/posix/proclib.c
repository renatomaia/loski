#include "proclib.h"
#include "procmgr.h"

/*
 * See:
 * http://www.steve.org.uk/Reference/Unix/faq_2.html
 * http://www.linuxjournal.com/article/2121?page=0,1
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <lauxlib.h>


LOSKIDRV_API loski_ErrorCode loskiP_initdrv (loski_ProcDriver *drv)
{
	loskiP_initprocmgr(NULL, NULL);
	return 0;
}

LOSKIDRV_API void loskiP_freedrv (loski_ProcDriver *drv)
{
	/* nothing to do */
}

#define sortvalues(A,B,T)	if (A>B) { T=A; A=B; B=T; }
#define closerange(A,B,T)	for (T=A+1; T<B; ++T) close(T)

static void closefds(int fd1, int fd2, int fd3, int lim)
{
	int i = -1;
	sortvalues(fd1, fd2, i);
	sortvalues(fd2, fd3, i);
	sortvalues(fd1, fd2, i);
	closerange( 2 , fd1, i);
	closerange(fd1, fd2, i);
	closerange(fd2, fd3, i);
	closerange(fd3, lim, i);
}

#define STDIN_FD	0
#define STDOUT_FD	1
#define STDERR_FD	2

static int setupstdfd(int ifd, int ofd, int efd)
{
	if (ifd != STDIN_FD) {
		/* backup stdin */
		if (ofd == STDIN_FD || efd == STDIN_FD) {
			int tfd = dup(STDIN_FD);
			if (tfd == -1) return 0;
			if (ofd == STDIN_FD) ofd = tfd;
			if (efd == STDIN_FD) efd = tfd;
		}
		/* replace stdin */
		if (dup2(ifd, STDIN_FD) != STDIN_FD) return 0;
	}
	if (ofd != STDOUT_FD) {
		/* backup stdout */
		if (efd == STDOUT_FD) {
			efd = dup(STDOUT_FD);
			if (efd == -1) return 0;
		}
		/* replace stdin */
		if (dup2(ofd, STDOUT_FD) != STDOUT_FD) return 0;
	}
	if ((efd != STDERR_FD) && (dup2(efd, STDERR_FD) != STDERR_FD)) return 0;
	if (ifd > 2) if (close(ifd)) return 0;
	if (ofd > 2) if (close(ofd)) return 0;
	if (efd > 2) if (close(efd)) return 0;
	return 1;
}

static int execvep(const char *file,
                   char *const argv[],
                   char *const envp[],
                   char *path)
{
	if (!strchr(file, '/')) {
		const char *prefix = getenv("PATH");
		
		size_t namelen = strlen(file);
		while(prefix && *prefix != '\0')
		{
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

static int resetinheritableprops ()
{
	// TODO:
	return 1;
}


LOSKIDRV_API int loskiP_checkargs (loski_ProcDriver *drv,
                                   loski_ProcArgInfo *info,
                                   loski_ProcArgFunc getter,
                                   void *data, int count,
                                   size_t *size)
{
	*size = (count+1)*sizeof(const char *); /* args + NULL */
	return 0;
}

LOSKIDRV_API void loskiP_initargs (loski_ProcDriver *drv,
                                   loski_ProcArgInfo *info,
                                   loski_ProcArgFunc getter,
                                   void *data, int count,
                                   void *args, size_t size)
{
	const char **argv = (const char **)args;
	int i;
	for (i = 0; i < count; ++i) argv[i] = getter(data, i);
	argv[count] = NULL;
}

LOSKIDRV_API int loskiP_checkenv (loski_ProcDriver *drv,
                                  loski_ProcEnvInfo *count,
                                  loski_ProcEnvFunc getter,
                                  void *data,
                                  size_t *size)
{
	size_t chars = 0;
	*count = 0;
	const char *name, *value; 
	while ((value = getter(data, &name))) {
		for (; *name; ++name, ++chars) if (*name == '=') return LOSKI_ERRINVALID;
		chars += strlen(value);
		++(*count);
	}
	*size = ((*count)+1)*(sizeof(char *)) /* the 'envl' array + NULL */
	      + (chars+2*(*count))*(sizeof(char)); /* #key + #value + '=' + '\0' */
	return 0;
}

LOSKIDRV_API void loskiP_initenv (loski_ProcDriver *drv,
                                  loski_ProcEnvInfo *count,
                                  loski_ProcEnvFunc getter,
                                  void *data,
                                  void *envlist, size_t size)
{
	char **envl = (char **)envlist;
	char *str = (char *)(envlist + ((*count)+1)*sizeof(char *));
	int i = 0;
	const char *c, *value; 
	while ((value = getter(data, &c))) {
		envl[i++] = str; /* put string in 'envl' array */
		while (*c) *str++ = *c++; /* copy key to string, excluding '\0' */
		*str++ = '=';
		c = value;
		while ((*str++ = *c++)); /* copy value to string, including '\0' */
	}
	envl[i] = NULL; /* put NULL to mark the end of 'envl' array */
}

LOSKIDRV_API loski_ErrorCode loskiP_initproc (loski_ProcDriver *drv,
                                              loski_Process *proc,
                                              const char *binpath,
                                              const char *runpath,
                                              void *argv,
                                              void *envl,
                                              loski_ProcStream *stdin,
                                              loski_ProcStream *stdout,
                                              loski_ProcStream *stderr)
{
	int err;
	char pathbuf[PATH_MAX];
	int ifd = stdin ? *stdin : 0;
	int ofd = stdout ? *stdout : 1;
	int efd = stderr ? *stderr : 2;
	int mfd = sysconf(_SC_OPEN_MAX);
	if (ifd==-1 || ofd==-1 || efd==-1 || mfd==-1) return LOSKI_ERRUNEXPECTED;
	loskiP_lockprocmgr();
	err = !loskiP_incprocmgr();
	loskiP_unlockprocmgr();
	if (err) return LOSKI_ERRNOMEMORY;
	if (runpath) {
		if (getcwd(pathbuf, PATH_MAX) == NULL) {
			switch (errno) {
				case EINVAL:
				case ERANGE:
				case EACCES:
				case ENOMEM: return LOSKI_ERRUNEXPECTED;
			}
			return LOSKI_ERRUNSPECIFIED;
		}
		if (chdir(runpath)) {
			switch (errno) {
				case EACCES: return LOSKI_ERRDENIED;
				case ELOOP:
				case ENAMETOOLONG: return LOSKI_ERRTOOMUCH;
				case ENOENT:
				case ENOTDIR: return LOSKI_ERRINVALID;
			}
			return LOSKI_ERRUNSPECIFIED;
		}
	}
	loskiP_lockprocmgr();
	proc->pid = fork();
	proc->status = 0;
	if (proc->pid == -1) {
		switch (errno) {
			case EAGAIN: err = LOSKI_ERRNORESOURCES; break;
			case ENOMEM: err = LOSKI_ERRNOMEMORY; break;
			default: err = LOSKI_ERRUNSPECIFIED;
		}
	} else if (proc->pid > 0) {
		loskiP_putprocmgr(proc);
		err = 0;
	} else {
		/* child process */
		loskiP_unlockprocmgr();
		closefds(ifd, ofd, efd, mfd);
		if (setupstdfd(ifd, ofd, efd) && resetinheritableprops()) {
			if (!envl) execvp(binpath, (char *const *)argv);
			else execvep(binpath, (char *const *)argv, (char *const *)envl, pathbuf);
		}
		_exit(0xff);
		err = LOSKI_ERRUNEXPECTED;  /* avoid warning */
	}
	loskiP_unlockprocmgr();
	if (runpath && chdir(pathbuf)) err = LOSKI_ERRUNEXPECTED;
	return err;
}

LOSKIDRV_API loski_ErrorCode loskiP_getprocstat (loski_ProcDriver *drv,
                                                 loski_Process *proc,
                                                 loski_ProcStatus *status)
{
	int err = 0;
	if (proc->pid != 0) {
		pid_t waitres;
		loskiP_lockprocmgr();
		waitres = waitpid(proc->pid, &proc->status, WNOHANG|WCONTINUED|WUNTRACED);
		if (waitres != -1) {
			if (proc->status == 0) {
				*status = (waitres == proc->pid) ? LOSKI_PROCSTAT_DEAD
				                                 : LOSKI_PROCSTAT_RUNNING;
			} else if (WIFEXITED(proc->status)) {
				*status = LOSKI_PROCSTAT_DEAD;
			} else if (WIFSIGNALED(proc->status)) {
				*status = LOSKI_PROCSTAT_DEAD;
			} else if (WIFSTOPPED(proc->status)) {
				*status = LOSKI_PROCSTAT_SUSPENDED;
			} else if (WIFCONTINUED(proc->status)) {
				*status = LOSKI_PROCSTAT_RUNNING;
			}
			if (*status == LOSKI_PROCSTAT_DEAD) {
				loskiP_delprocmgr(proc);
				proc->pid = 0;
			}
		}
		else switch (errno) {
			case EINTR:
			case ECHILD:
			case EINVAL: err = LOSKI_ERRUNEXPECTED; break;
			default: err = LOSKI_ERRUNSPECIFIED;
		}
		loskiP_unlockprocmgr();
	}
	else *status = LOSKI_PROCSTAT_DEAD;
	return err;
}

LOSKIDRV_API loski_ErrorCode loskiP_getprocexit (loski_ProcDriver *drv,
                                                 loski_Process *proc,
                                                 int *code)
{
	if (proc->pid == 0) {
		if (WIFEXITED(proc->status)) {
			*code = WEXITSTATUS(proc->status);
			return 0;
		} else if (WIFSIGNALED(proc->status)) {
			return LOSKI_ERRABORTED;
		}
	}
	return LOSKI_ERRUNFULFILLED;
}

LOSKIDRV_API loski_ErrorCode loskiP_killproc (loski_ProcDriver *drv,
                                              loski_Process *proc)
{
	if (proc->pid != 0) if (kill(proc->pid, SIGKILL) == -1) switch (errno) {
		case EPERM: return LOSKI_ERRDENIED;
		case ESRCH:
		case EINVAL: return LOSKI_ERRUNEXPECTED;
		default: return LOSKI_ERRUNSPECIFIED;
	}
	return 0;
}

LOSKIDRV_API void loskiP_freeproc (loski_ProcDriver *drv,
                                   loski_Process *proc)
{
	if (proc->pid > 0) {
		loskiP_lockprocmgr();
		loskiP_delprocmgr(proc);
		loskiP_unlockprocmgr();
		proc->pid = 0;
	}
}

LOSKIDRV_API int loskiP_luafilestream (lua_State *L, int idx,
                                       loski_ProcStream *fd)
{
	FILE **fp = (FILE **)luaL_testudata(L, idx, LUA_FILEHANDLE);
	if (*fp == NULL) return 0;
	*fd = fileno(*fp);
	return 1;
}
