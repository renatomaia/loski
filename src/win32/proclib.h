#ifndef proclib_h
#define proclib_h


#include <windows.h>

struct loski_Process {
	PROCESS_INFORMATION pi;
	DWORD exitcode;
};

typedef int loski_ProcArgInfo;
typedef size_t loski_ProcEnvInfo;

#include "proclibapi.h"

#endif
