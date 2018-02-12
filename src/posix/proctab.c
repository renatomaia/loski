#include "luaosi/proctab.h"


#include <string.h>  /* memcpy */


#define RATIO_EMPTY 1
#define RATIO_INC 2
#define RATIO_FULL 3
#define RATIO_BASE 4


#define calcsize(C)	(C*sizeof(losi_Process *))
#define calchash(P,C) ((P)%(C))


static void addentry(losi_Process **table,
                     size_t pos,
                     losi_Process *proc)
{
	losi_Process **place = table+pos;
	for (; *place; place=&((*place)->next));
	*place = proc;
	proc->next = NULL;
}

static losi_Process *removeentry(losi_Process **table,
                                 size_t pos,
                                 losi_Process *proc)
{
	losi_Process **place = table+pos;
	for (; *place; place=&((*place)->next)) if (*place == proc) {
		*place = proc->next;
		proc->next = NULL;
		return *place;
	}
	return NULL;
}

static void rehashtable(losi_ProcTable *tab, size_t capacity)
{
	size_t i;
	for (i = tab->capacity; i < capacity; ++i) tab->table[i] = NULL;
	for (i = 0; i < tab->capacity; ++i) {
		losi_Process *proc = tab->table[i];
		while (proc) {
			size_t pos = calchash(proc->pid, capacity);
			if (pos != i) {
				losi_Process *next = removeentry(tab->table, i, proc);
				addentry(tab->table, pos, proc);
				proc = next;
			}
			else proc = proc->next;
		}
	}
	tab->capacity = capacity;
}


void losiP_initproctab(losi_ProcTable *tab, losi_Alloc af, void *aud)
{
	size_t i;
	tab->allocf = af;
	tab->allocud = aud;
	tab->count = 0;
	tab->capacity = LOSI_PROCTABMINSZ;
	tab->table = tab->mintab;
	for (i=0; i<tab->capacity; ++i) tab->table[i] = NULL;
}

int losiP_incproctab(losi_ProcTable *tab)
{
	size_t capacity = tab->capacity;
	if (tab->count >= capacity*RATIO_FULL/RATIO_BASE) {
		losi_Process **memo;
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
	} else if (tab->table != tab->mintab &&
	           tab->count < capacity*RATIO_EMPTY/RATIO_BASE) {
		size_t newcapacity = capacity;
		do { newcapacity /= RATIO_INC; }
		while (newcapacity > LOSI_PROCTABMINSZ &&
		       tab->count < newcapacity*RATIO_EMPTY/RATIO_BASE);
		rehashtable(tab, newcapacity);
		if (tab->capacity <= LOSI_PROCTABMINSZ) {
			memcpy(tab->mintab, tab->table, calcsize(tab->capacity));
			tab->allocf(tab->allocud, tab->table, calcsize(capacity), 0);
			tab->table = tab->mintab;
		} else {
			tab->table = tab->allocf(tab->allocud, tab->table,
			                         calcsize(capacity),
			                         calcsize(tab->capacity));
		}
	}
	return 1;
}

void losiP_putproctab(losi_ProcTable *tab, losi_Process *proc)
{
	addentry(tab->table, calchash(proc->pid, tab->capacity), proc);
	++(tab->count);
}

void losiP_delproctab(losi_ProcTable *tab, losi_Process *proc)
{
	removeentry(tab->table, calchash(proc->pid, tab->capacity), proc);
	--(tab->count);
}

losi_Process *losiP_findproctab(losi_ProcTable *tab, pid_t pid)
{
	losi_Process *proc = tab->table[calchash(pid, tab->capacity)];
	for (; proc; proc=proc->next) if (proc->pid==pid) break;
	return proc;
}

int losiP_emptyproctab(losi_ProcTable *tab)
{
	return tab->count == 0;
}
