#ifndef proclib_h
#define proclib_h


#include <windows.h>

typedef void losi_ProcDriver;
typedef struct losi_Process {
	PROCESS_INFORMATION pi;
	DWORD exitcode;
} losi_Process;
typedef int losi_ProcArgInfo;
typedef size_t losi_ProcEnvInfo;

#include "proclibapi.h"

#endif
