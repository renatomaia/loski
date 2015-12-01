#include "proclib.h"
#include "procmgr.h"

/*
 * See:
 * http://www.steve.org.uk/Reference/Unix/faq_2.html
 * http://www.linuxjournal.com/article/2121?page=0,1
 */

#include <loskierr.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>


LOSKIDRV_API int loskiP_initdrv (loski_ProcDriver *drv)
{
	loskiP_initprocmgr(NULL, NULL);
	return 0;
}

LOSKIDRV_API void loskiP_freedrv (loski_ProcDriver *drv)
{
	/* nothing to do */
}


static int closefds(int fd1, int fd2, int fd3, int lim)
{
	int i = -1;
	if (fd1 > fd2) { i = fd1; fd1 = fd2; fd2 = i; }
	if (fd2 > fd3) { i = fd2; fd2 = fd3; fd3 = i; }
	if (i != -1 && fd1 > fd2) { i = fd1; fd1 = fd2; fd2 = i; }
	for (i = 3; i < fd1; ++i) if (close(i)) return 0;
	for (i = fd1+1; i < fd2; ++i) if (close(i)) return 0;
	for (i = fd2+1; i < fd3; ++i) if (close(i)) return 0;
	for (i = fd3+1; i < lim; ++i) if (close(i)) return 0;
	return 1;
}

static int setstdfd(int stdfd, int fd)
{
	return (fd==stdfd || (dup2(fd,stdfd)==fd && (fd<3 || close(fd)!=EOF)));
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

LOSKIDRV_API int loski_initproc (loski_ProcDriver *drv,
                                 loski_Process *proc,
                                 const char *binpath,
                                 const char *runpath,
                                 void *argv,
                                 void *envl,
                                 FILE *stdin,
                                 FILE *stdout,
                                 FILE *stderr)
{
	int err;
	char pathbuf[PATH_MAX];
	int ifd = stdin ? fileno(stdin) : 0;
	int ofd = stdout ? fileno(stdout) : 1;
	int efd = stderr ? fileno(stderr) : 2;
	int mfd = sysconf(_SC_OPEN_MAX);
	if (ifd==-1 || ofd==-1 || efd==-1 || mfd==-1) return LOSKI_ERRUNEXPECTED;
	if (ifd<=2 || ofd<=2 || efd<=2) return LOSKI_ERRUNSUPPORTED; // TODO: FIX IT!
	if (!loskiP_incprocmgr()) return LOSKI_ERRNOMEMORY;
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
	fflush(stdout);
	fflush(stderr);
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
		if (closefds(ifd, ofd, efd, mfd) &&
		    setstdfd(0, ifd) && setstdfd(1, ofd) && setstdfd(2, efd) &&
		    resetinheritableprops()) {
			if (!envl) execvp(binpath, (char *const *)argv);
			else execvep(binpath, (char *const *)argv, (char *const *)envl, pathbuf);
		}
		_exit(0xff);
		err = LOSKI_ERRUNEXPECTED;  /* avoid warning */
	}
	loskiP_unlockprocmgr();
	if (chdir(pathbuf)) return LOSKI_ERRUNEXPECTED;
	return err;
}

LOSKIDRV_API int loskiP_getprocstat (loski_ProcDriver *drv,
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
		else err = errno;
		loskiP_unlockprocmgr();
	}
	else *status = LOSKI_PROCSTAT_DEAD;
	return err;
}

LOSKIDRV_API int loskiP_getprocexit (loski_ProcDriver *drv,
                                     loski_Process *proc,
                                     int *code)
{
	if ( (proc->pid == 0) && WIFEXITED(proc->status) ) {
		*code = WEXITSTATUS(proc->status);
		return 0;
	}
	return LOSKI_ERRUNFULFILLED;
}

LOSKIDRV_API int loskiP_killproc (loski_ProcDriver *drv,
                                  loski_Process *proc)
{
	if (proc->pid != 0) {
		if (kill(proc->pid, SIGKILL) == -1) return errno;
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
