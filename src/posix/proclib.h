#ifndef proclib_h
#define proclib_h


#include <sys/types.h>

struct loski_Process {
	pid_t pid;
	int status;
	struct loski_Process *next;
};

#include "proclibapi.h"

#endif
