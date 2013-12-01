#ifndef proctab_h
#define proctab_h


#include "proclib.h"

typedef struct loski_ProcTable {
	loski_Process **table;
	size_t capacity;
	size_t count;
} loski_ProcTable;

LOSKIDRV_API int loski_proctabinit(loski_ProcTable *tab, size_t capacity);

LOSKIDRV_API void loski_proctabclose(loski_ProcTable *tab);

LOSKIDRV_API loski_Process *loski_proctabput(loski_ProcTable *tab,
                                          loski_Process *proc);

LOSKIDRV_API loski_Process *loski_proctabdel(loski_ProcTable *tab, pid_t pid);

/* LOSKIDRV_API loski_Process *loski_proctabget(loski_ProcTable *tab, pid_t pid); */


#endif
