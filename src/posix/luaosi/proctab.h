#ifndef luaosi_proctab_h
#define luaosi_proctab_h


#include "luaosi/proclib.h"

#define LOSI_PROCTABMINSZ	4

typedef struct losi_ProcTable {
	losi_Alloc allocf;
	void *allocud;
	losi_Process *mintab[LOSI_PROCTABMINSZ];
	losi_Process **table;
	size_t capacity;
	size_t count;
} losi_ProcTable;

void losiP_initproctab(losi_ProcTable *tab, losi_Alloc af, void *aud);

int losiP_incproctab(losi_ProcTable *tab);

void losiP_putproctab(losi_ProcTable *tab, losi_Process *proc);

void losiP_delproctab(losi_ProcTable *tab, losi_Process *proc);

losi_Process *losiP_findproctab(losi_ProcTable *tab, pid_t pid);

int losiP_emptyproctab(losi_ProcTable *tab);


#endif
