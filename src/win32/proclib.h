#ifndef proclib_h
#define proclib_h


#include <windows.h>

typedef void loski_ProcDriver;
typedef struct loski_Process {
	PROCESS_INFORMATION pi;
	DWORD exitcode;
} loski_Process;
typedef int loski_ProcArgInfo;
typedef size_t loski_ProcEnvInfo;

#include "proclibapi.h"

#endif
