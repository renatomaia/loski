#ifndef proclib_h
#define proclib_h


#include <sys/types.h>

typedef void loski_ProcDriver;

typedef struct loski_Process {
	pid_t pid;
	int status;
	struct loski_Process *next;
} loski_Process;

typedef char loski_ProcArgInfo;

typedef int loski_ProcEnvInfo;

#include "proclibapi.h"


#endif
