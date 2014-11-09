#ifndef proclib_h
#define proclib_h


#include <sys/types.h>

struct loski_Process {
	pid_t pid;
	int status;
	struct loski_Process **place;
	struct loski_Process *next;
};

typedef int loski_ProcArgInfo;
typedef size_t loski_ProcEnvInfo;

#include "proclibapi.h"

#endif
