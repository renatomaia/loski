#include <stdio.h>

#include <assert.h>
#include <string.h>

#include "luaosi/proctab.h"

#define calcsize(C)	(C*sizeof(loski_Process *))

#define MAXPROC 24
#define MAXSIZE 32
#define MAXPID (MAXPROC*MAXSIZE)
#define UNUSED ((char)0xff)

static size_t memsz = 0;
static char memory[calcsize(MAXSIZE)];
static char *allocmem = NULL;

static void *allocf (void *ud, void *ptr, size_t osize, size_t nsize) {
	size_t i;
	size_t oini = calcsize(MAXSIZE)-osize;
	assert(ud == &memsz);
	assert(ptr == allocmem);
	assert((ptr == NULL && osize == 0 && memsz == 0) || (ptr && osize == memsz));
	assert(nsize >= 0);
	for (i=0; i<oini; ++i) assert(memory[i] == UNUSED);
	if (nsize == 0) {
		for (i=0; i<calcsize(MAXSIZE); ++i) memory[i] = UNUSED;
		memsz = 0;
		allocmem = NULL;
	} else if (nsize <= calcsize(MAXSIZE)) {
		size_t nini = calcsize(MAXSIZE)-nsize;
		memmove(memory+nini, memory+oini, osize<nsize ? osize : nsize);
		for (i=0; i<nini; ++i) memory[i] = UNUSED;
		memsz = nsize;
		allocmem = memory+nini;
		return allocmem;
	}
	return NULL;
}

static loski_Process p[MAXPROC];

static void checkvalues(loski_ProcTable *t, size_t s, size_t n, size_t gap)
{
	size_t i;
	for (i=0; i<MAXPID; ++i) {
		size_t o = i/gap;
		if (i%gap==0 && s<=o && o<n) assert(loskiP_findproctab(t, i) == p+o);
		else assert(loskiP_findproctab(t, i) == NULL);
	}
}

static void checkmemo()
{
	size_t i;
	for (i=0; i<calcsize(MAXSIZE)-memsz; ++i) assert(memory[i] == UNUSED);
}
/*
static void printtab(loski_ProcTable *tab)
{
	size_t i;
	for (i = 0; i < tab->capacity; ++i) {
		printf("\t[%ld] =", i);
		loski_Process *proc = tab->table[i];
		while (proc) {
			printf(" %d", proc->pid);
			proc = proc->next;
		}
		printf("\n");
	}
}
*/
int main (int argc, const char* argv[])
{
	size_t n=MAXPROC, i, gap, prevgap;
	loski_ProcTable t;

	printf("posix/proctab.c "); fflush(stdout);

	for (i=0; i<calcsize(MAXSIZE); ++i) memory[i] = UNUSED;

	loskiP_initproctab(&t, allocf, &memsz);

	for (gap=1, prevgap=1; gap<=MAXSIZE; i=gap, gap=gap+prevgap, prevgap=i) {
		printf("."); fflush(stdout);
		assert(memsz == 0);
		checkmemo();
		for (i=0; i<n; ++i) {
			p[i].pid = i*gap;
			assert(loskiP_incproctab(&t));
			loskiP_putproctab(&t, p+i);
			if (i+1 < LOSKI_PROCTABMINSZ) assert(memsz == 0);
			else assert(memsz > 0);
			checkvalues(&t, 0, i+1, gap);
			checkmemo();
		}

		assert(memsz == calcsize(MAXSIZE));
		assert(!loskiP_incproctab(&t));
		assert(memsz == calcsize(MAXSIZE));
		checkvalues(&t, 0, n, gap);
		for (i=0; i<n; ++i) {
			loskiP_delproctab(&t, p+i);
			assert(p[i].pid == i*gap);
			assert(loskiP_findproctab(&t, i*gap) == NULL);
			assert(loskiP_incproctab(&t));
			checkvalues(&t, i+1, n, gap);
			checkmemo();
		}
		assert(memsz == 0);
		for (i=0; i<MAXPID; ++i) assert(loskiP_findproctab(&t, i) == NULL);
	}

	assert(memsz == 0);
	checkmemo();

	printf(" OK\n");

	return 0;
}
