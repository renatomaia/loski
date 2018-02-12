#include "luaosi/proclib.h"
#include "luaosi/procmgr.h"

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


LOSIDRV_API losi_ErrorCode losiP_initdrv (losi_ProcDriver *drv)
{
	losiP_initprocmgr(NULL, NULL);
	return LOSI_ERRNONE;
}

LOSIDRV_API void losiP_freedrv (losi_ProcDriver *drv)
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


LOSIDRV_API int losiP_checkargs (losi_ProcDriver *drv,
                                 losi_ProcArgInfo *info,
                                 losi_ProcArgFunc getter,
                                 void *data, int count,
                                 size_t *size)
{
	*size = (count+1)*sizeof(const char *); /* args + NULL */
	return 0;
}

LOSIDRV_API void losiP_initargs (losi_ProcDriver *drv,
                                 losi_ProcArgInfo *info,
                                 losi_ProcArgFunc getter,
                                 void *data, int count,
                                 void *args, size_t size)
{
	const char **argv = (const char **)args;
	int i;
	for (i = 0; i < count; ++i) argv[i] = getter(data, i);
	argv[count] = NULL;
}

LOSIDRV_API int losiP_checkenv (losi_ProcDriver *drv,
                                losi_ProcEnvInfo *count,
                                losi_ProcEnvFunc getter,
                                void *data,
                                size_t *size)
{
	size_t chars = 0;
	*count = 0;
	const char *name, *value; 
	while ((value = getter(data, &name))) {
		for (; *name; ++name, ++chars) if (*name == '=') return LOSI_ERRINVALID;
		chars += strlen(value);
		++(*count);
	}
	*size = ((*count)+1)*(sizeof(char *)) /* the 'envl' array + NULL */
	      + (chars+2*(*count))*(sizeof(char)); /* #key + #value + '=' + '\0' */
	return 0;
}

LOSIDRV_API void losiP_initenv (losi_ProcDriver *drv,
                                losi_ProcEnvInfo *count,
                                losi_ProcEnvFunc getter,
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

LOSIDRV_API losi_ErrorCode losiP_initproc (losi_ProcDriver *drv,
                                           losi_Process *proc,
                                           const char *binpath,
                                           const char *runpath,
                                           void *argv,
                                           void *envl,
                                           losi_ProcStream *stdin,
                                           losi_ProcStream *stdout,
                                           losi_ProcStream *stderr)
{
	int err;
	char pathbuf[PATH_MAX];
	int ifd = stdin ? *stdin : 0;
	int ofd = stdout ? *stdout : 1;
	int efd = stderr ? *stderr : 2;
	int mfd = sysconf(_SC_OPEN_MAX);
	if (ifd==-1 || ofd==-1 || efd==-1 || mfd==-1) return LOSI_ERRUNEXPECTED;
	losiP_lockprocmgr();
	err = !losiP_incprocmgr();
	losiP_unlockprocmgr();
	if (err) return LOSI_ERRNOMEMORY;
	if (runpath) {
		if (getcwd(pathbuf, PATH_MAX) == NULL) {
			switch (errno) {
				case EINVAL:
				case ERANGE:
				case EACCES:
				case ENOMEM: return LOSI_ERRUNEXPECTED;
			}
			return LOSI_ERRUNSPECIFIED;
		}
		if (chdir(runpath)) {
			switch (errno) {
				case EACCES: return LOSI_ERRDENIED;
				case ELOOP:
				case ENAMETOOLONG: return LOSI_ERRTOOMUCH;
				case ENOENT:
				case ENOTDIR: return LOSI_ERRINVALID;
			}
			return LOSI_ERRUNSPECIFIED;
		}
	}
	losiP_lockprocmgr();
	proc->pid = fork();
	proc->status = 0;
	proc->pipe[0] = -1;
	proc->pipe[1] = -1;
	proc->piperefs = 0;
	if (proc->pid == -1) {
		switch (errno) {
			case EAGAIN: err = LOSI_ERRNORESOURCES; break;
			case ENOMEM: err = LOSI_ERRNOMEMORY; break;
			default: err = LOSI_ERRUNSPECIFIED;
		}
	} else if (proc->pid > 0) {
		losiP_putprocmgr(proc);
		err = 0;
	} else {
		/* child process */
		losiP_unlockprocmgr();
		closefds(ifd, ofd, efd, mfd);
		if (setupstdfd(ifd, ofd, efd) && resetinheritableprops()) {
			if (!envl) execvp(binpath, (char *const *)argv);
			else execvep(binpath, (char *const *)argv, (char *const *)envl, pathbuf);
		}
		_exit(0xff);
		err = LOSI_ERRUNEXPECTED;  /* avoid warning */
	}
	losiP_unlockprocmgr();
	if (runpath && chdir(pathbuf)) err = LOSI_ERRUNEXPECTED;
	return err;
}

LOSIDRV_API losi_ErrorCode losiP_getprocstat (losi_ProcDriver *drv,
                                              losi_Process *proc,
                                              losi_ProcStatus *status)
{
	int err = 0;
	losiP_lockprocmgr();
	if (proc->pid != 0) {
		pid_t waitres;
		waitres = waitpid(proc->pid, &proc->status, WNOHANG|WCONTINUED|WUNTRACED);
		if (waitres != -1) {
			if (proc->status == 0) {
				*status = (waitres == proc->pid) ? LOSI_PROCSTAT_DEAD
				                                 : LOSI_PROCSTAT_RUNNING;
			} else if (WIFEXITED(proc->status)) {
				*status = LOSI_PROCSTAT_DEAD;
			} else if (WIFSIGNALED(proc->status)) {
				*status = LOSI_PROCSTAT_DEAD;
			} else if (WIFSTOPPED(proc->status)) {
				*status = LOSI_PROCSTAT_SUSPENDED;
			} else if (WIFCONTINUED(proc->status)) {
				*status = LOSI_PROCSTAT_RUNNING;
			}
			if (*status == LOSI_PROCSTAT_DEAD) {
				losiP_delprocmgr(proc);
				proc->pid = 0;
			}
		}
		else switch (errno) {
			case EINTR:
			case ECHILD:
			case EINVAL: err = LOSI_ERRUNEXPECTED; break;
			default: err = LOSI_ERRUNSPECIFIED;
		}
	}
	else *status = LOSI_PROCSTAT_DEAD;
	losiP_unlockprocmgr();
	return err;
}

LOSIDRV_API losi_ErrorCode losiP_getprocexit (losi_ProcDriver *drv,
                                              losi_Process *proc,
                                              int *code)
{
	if (proc->pid == 0) {
		if (WIFEXITED(proc->status)) {
			*code = WEXITSTATUS(proc->status);
			return LOSI_ERRNONE;
		} else if (WIFSIGNALED(proc->status)) {
			return LOSI_ERRABORTED;
		}
	}
	return LOSI_ERRUNFULFILLED;
}

LOSIDRV_API losi_ErrorCode losiP_killproc (losi_ProcDriver *drv,
                                           losi_Process *proc)
{
	losi_ErrorCode err = LOSI_ERRNONE;
	losiP_lockprocmgr();
	if (proc->pid != 0) if (kill(proc->pid, SIGKILL) == -1) switch (errno) {
		case EPERM: err = LOSI_ERRDENIED; break;
		case ESRCH:
		case EINVAL: err = LOSI_ERRUNEXPECTED; break;
		default: err = LOSI_ERRUNSPECIFIED; break;
	}
	losiP_unlockprocmgr();
	return err;
}

LOSIDRV_API void losiP_freeproc (losi_ProcDriver *drv,
                                 losi_Process *proc)
{
	losiP_lockprocmgr();
	if (proc->pid != 0) {
		losiP_delprocmgr(proc);
		proc->pid = 0;
	}
	losiP_unlockprocmgr();
}

LOSIDRV_API int losiP_getluafileprocstrm (void *udata, losi_ProcStream *fd)
{
	FILE **fp = (FILE **)udata;
	if (*fp == NULL) return 0;
	*fd = fileno(*fp);
	return 1;
}

LOSIDRV_API losi_ErrorCode losiP_getprocevtsrc (void *udata, int newref,
                                                losi_EventSource *src,
                                                losi_EventFlags evtflags)
{
	losi_ErrorCode err = LOSI_ERRNONE;
	losi_Process *proc = (losi_Process *)udata;
	if (evtflags & LOSI_EVTFLAGS_OUTPUT) return LOSI_ERRUNSUPPORTED;
	losiP_lockprocmgr();
	if (proc->pipe[1] != -1 || (proc->pid && newref && pipe(proc->pipe) == 0)) {
		if (newref) ++(proc->piperefs);
		*src = proc->pipe[1];
	} else {
		err = LOSI_ERRCLOSED;
	}
	losiP_unlockprocmgr();
	return err;
}

#define closefd(F)	while (close(F) != 0 && errno == EINTR)

LOSIDRV_API void losiP_freeprocevtsrc (void *udata)
{
	losi_Process *proc = (losi_Process *)udata;
	if (--(proc->piperefs) == 0) {
		losiP_lockprocmgr();
		if (proc->pipe[0] != -1) {
			closefd(proc->pipe[0]);
			proc->pipe[0] = -1;
		}
		losiP_unlockprocmgr();
		closefd(proc->pipe[1]);
		proc->pipe[1] = -1;
	}
}
