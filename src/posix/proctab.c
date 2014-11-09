#include "proctab.h"

#include <stdio.h>
#include <stdlib.h>

#define FULL_RATIO .75
#define EMPTY_RATIO .25
#define INC_RATIO 2

#define calchash(pid, capacity) (((pid)-1)%(capacity))


static loski_Process **newtable(size_t capacity)
{
	loski_Process **table = (loski_Process**) malloc(capacity*sizeof(loski_Process*));
	if (table) {
		size_t i;
		for (i = 0; i<capacity; ++i) table[i] = NULL;
	}
	return table;
}

static void addtotable(loski_Process **table, size_t capacity, loski_Process *proc)
{
	loski_Process **place = table+calchash(proc->pid, capacity);
	for (; *place; place=&((*place)->next));
	*place = proc;
	proc->place = place;
	proc->next = NULL;
}

static int rehashtable(loski_ProcTable *tab, size_t capacity)
{
	size_t i;
	loski_Process **table = newtable(capacity);
	if (table == NULL) return 1;
	for (i = 0; i < tab->capacity; ++i) {
		loski_Process *proc = tab->table[i];
		tab->table[i] = NULL;
		while (proc) {
			loski_Process *next = proc->next;
			addtotable(table, capacity, proc);
			proc = next;
		}
	}
	free(tab->table);
	tab->table = table;
	tab->capacity = capacity;
	return 0;
}


int loski_proctabinit(loski_ProcTable *tab, size_t capacity)
{
	tab->count = 0;
	tab->capacity = capacity;
	tab->table = newtable(capacity);
	if (tab->table == NULL) return -1;
	return 0;
}

void loski_proctabclose(loski_ProcTable *tab)
{
	size_t i;
	for (i = 0; i < tab->capacity; ++i) {
		loski_Process* proc = tab->table[i];
		while (proc) {
			loski_Process* next = proc->next;
			proc->place = NULL;
			proc->next = NULL;
			proc = next;
		}
	}
	free(tab->table);
}

loski_Process *loski_proctabput(loski_ProcTable *tab, loski_Process *proc)
{
	size_t capacity = tab->capacity;
	if (tab->count >= capacity*FULL_RATIO) {
		int error = rehashtable(tab, capacity*INC_RATIO);
		if (error) return NULL;
	}
	addtotable(tab->table, tab->capacity, proc);
	++(tab->count);
	return proc;
}

loski_Process *loski_proctabdel(loski_ProcTable *tab, loski_Process *proc)
{
	loski_Process **place = proc->place;
	if (place) {
		*place = proc->next;
		proc->place = NULL;
		proc->next = NULL;
		--(tab->count);
		if (tab->count > 0 && tab->count < tab->capacity*EMPTY_RATIO)
			rehashtable(tab, tab->capacity/INC_RATIO);
		return proc;
	}
	return NULL;
}

loski_Process *loski_proctabget(loski_ProcTable *tab, pid_t pid)
{
	loski_Process *proc = tab->table[calchash(pid, tab->capacity)];
	for (; proc; proc=proc->next) if (proc->pid==pid) break;
	return proc;
}

int loski_proctabisempty(loski_ProcTable *tab)
{
	return tab->count == 0;
}
