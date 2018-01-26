#include "luaosi/proctab.h"


#include <string.h>  /* memcpy */


#define RATIO_EMPTY 1
#define RATIO_INC 2
#define RATIO_FULL 3
#define RATIO_BASE 4


#define calcsize(C)	(C*sizeof(loski_Process *))
#define calchash(P,C) ((P)%(C))


static void addentry(loski_Process **table,
                     size_t pos,
                     loski_Process *proc)
{
	loski_Process **place = table+pos;
	for (; *place; place=&((*place)->next));
	*place = proc;
	proc->next = NULL;
}

static loski_Process *removeentry(loski_Process **table,
                                  size_t pos,
                                  loski_Process *proc)
{
	loski_Process **place = table+pos;
	for (; *place; place=&((*place)->next)) if (*place == proc) {
		*place = proc->next;
		proc->next = NULL;
		return *place;
	}
	return NULL;
}

static void rehashtable(loski_ProcTable *tab, size_t capacity)
{
	size_t i;
	for (i = tab->capacity; i < capacity; ++i) tab->table[i] = NULL;
	for (i = 0; i < tab->capacity; ++i) {
		loski_Process *proc = tab->table[i];
		while (proc) {
			size_t pos = calchash(proc->pid, capacity);
			if (pos != i) {
				loski_Process *next = removeentry(tab->table, i, proc);
				addentry(tab->table, pos, proc);
				proc = next;
			}
			else proc = proc->next;
		}
	}
	tab->capacity = capacity;
}


void loskiP_initproctab(loski_ProcTable *tab, loski_Alloc af, void *aud)
{
	size_t i;
	tab->allocf = af;
	tab->allocud = aud;
	tab->count = 0;
	tab->capacity = LOSKI_PROCTABMINSZ;
	tab->table = tab->mintab;
	for (i=0; i<tab->capacity; ++i) tab->table[i] = NULL;
}

int loskiP_incproctab(loski_ProcTable *tab)
{
	size_t capacity = tab->capacity;
	if (tab->count >= capacity*RATIO_FULL/RATIO_BASE) {
		loski_Process **memo;
		capacity *= RATIO_INC;
		if (tab->table == tab->mintab) {
			memo = tab->allocf(tab->allocud, NULL, 0, calcsize(capacity));
			if (memo) memcpy(memo, tab->table, calcsize(tab->capacity));
			else return 0;
		} else {
			memo = tab->allocf(tab->allocud, tab->table,
			                   calcsize(tab->capacity),
			                   calcsize(capacity));
			if (!memo) return 0;
		}
		tab->table = memo;
		rehashtable(tab, capacity);
	}
	return 1;
}

void loskiP_putproctab(loski_ProcTable *tab, loski_Process *proc)
{
	addentry(tab->table, calchash(proc->pid, tab->capacity), proc);
	++(tab->count);
}

void loskiP_delproctab(loski_ProcTable *tab, loski_Process *proc)
{
	size_t capacity = tab->capacity;
	removeentry(tab->table, calchash(proc->pid, capacity), proc);
	--(tab->count);
	if (tab->table != tab->mintab && tab->count < capacity*RATIO_EMPTY/RATIO_BASE) {
		rehashtable(tab, capacity/RATIO_INC);
		if (tab->capacity == LOSKI_PROCTABMINSZ) {
			memcpy(tab->mintab, tab->table, calcsize(tab->capacity));
			tab->allocf(tab->allocud, tab->table, calcsize(capacity), 0);
			tab->table = tab->mintab;
		} else {
			tab->table = tab->allocf(tab->allocud, tab->table,
			                         calcsize(capacity),
			                         calcsize(tab->capacity));
		}
	}
}

loski_Process *loskiP_findproctab(loski_ProcTable *tab, pid_t pid)
{
	loski_Process *proc = tab->table[calchash(pid, tab->capacity)];
	for (; proc; proc=proc->next) if (proc->pid==pid) break;
	return proc;
}

int loskiP_emptyproctab(loski_ProcTable *tab)
{
	return tab->count == 0;
}
