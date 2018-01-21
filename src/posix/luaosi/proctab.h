#ifndef luaosi_proctab_h
#define luaosi_proctab_h


#include "luaosi/proclib.h"

#define LOSKI_PROCTABMINSZ	4

typedef struct loski_ProcTable {
	loski_Alloc allocf;
	void *allocud;
	loski_Process *mintab[LOSKI_PROCTABMINSZ];
	loski_Process **table;
	size_t capacity;
	size_t count;
} loski_ProcTable;

void loskiP_initproctab(loski_ProcTable *tab, loski_Alloc af, void *aud);

int loskiP_incproctab(loski_ProcTable *tab);

void loskiP_putproctab(loski_ProcTable *tab, loski_Process *proc);

void loskiP_delproctab(loski_ProcTable *tab, loski_Process *proc);

loski_Process *loskiP_findproctab(loski_ProcTable *tab, pid_t pid);

int loskiP_emptyproctab(loski_ProcTable *tab);


#endif
