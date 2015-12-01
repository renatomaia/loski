#include <stdio.h>

#include <assert.h>

#include "proctab.h"

#define calcsize(C)	(C*sizeof(loski_Process *))

#define MAXPROC 768
#define MAXSIZE 1024
#define MAXPID (MAXPROC*MAXSIZE)
#define UNUSED ((char)0xff)

static size_t memsz = 0;
static char memory[calcsize(MAXSIZE)];

static void *allocf (void *ud, void *ptr, size_t osize, size_t nsize) {
	size_t i;
	assert(ud == &memsz);
	assert(ptr == NULL || ptr == memory);
	assert((ptr == NULL && osize == 0 && memsz == 0) || (ptr && osize == memsz));
	assert(0 <= nsize);
	for (i=osize; i<calcsize(MAXSIZE); ++i) assert(memory[i] == UNUSED);
	for (i=nsize; i<calcsize(MAXSIZE); ++i) memory[i] = UNUSED;
	if (nsize == 0) {
		memsz = 0;
	} else if (nsize <= calcsize(MAXSIZE)) {
		memsz = nsize;
		return memory;
	}
	return NULL;
}

static loski_Process p[MAXPROC];

static void checkvalues(loski_ProcTable *t, size_t s, size_t n, size_t g)
{
	size_t i;
	for (i=0; i<MAXPID; ++i) {
		size_t o = i/g;
		if (i%g==0 && s<=o && o<n) assert(loskiP_findproctab(t, i) == p+o);
		else assert(loskiP_findproctab(t, i) == NULL);
	}
}

static void checkmemo()
{
	size_t i;
	for (i=memsz; i<calcsize(MAXSIZE); ++i) assert(memory[i] == UNUSED);
}

int main (int argc, const char* argv[])
{
	size_t n=MAXPROC, g, i;
	loski_ProcTable t;
	for (i=0; i<calcsize(MAXSIZE); ++i) memory[i] = UNUSED;

	loskiP_initproctab(&t, allocf, &memsz);

	for (g=1; g<=n*4/3; ++g) {

//printf("n=%ld, g=%ld\n", n, g);

		assert(memsz == 0);
		checkmemo();
		for (i=0; i<n; ++i) {
			p[i].pid = i*g;
			assert(loskiP_incproctab(&t));

//printf("  loskiP_putproctab(&t, p+%ld) /* {pid=%d} */\n", i, (int)(i*g));

			loskiP_putproctab(&t, p+i);
			if (i+1 < LOSKI_PROCTABMINSZ) assert(memsz == 0);
			else assert(memsz > 0);
			checkvalues(&t, 0, i+1, g);
			checkmemo();
		}
		assert(memsz == calcsize(MAXSIZE));
		assert(!loskiP_incproctab(&t));
		assert(memsz == calcsize(MAXSIZE));
		checkvalues(&t, 0, n, g);
		for (i=0; i<n; ++i) {

//printf("  loskiP_delproctab(&t, p+%ld) /* {pid=%d} */\n", i, (int)(i*g));

			loskiP_delproctab(&t, p+i);
			assert(p[i].pid == i*g);

//printf("  loskiP_findproctab(&t, %ld)=p+%ld /* {pid=%d}, i=%ld */\n", i, loskiP_findproctab(&t, i)-p, loskiP_findproctab(&t, i)->pid, i/g);

			assert(loskiP_findproctab(&t, i*g) == NULL);
			checkvalues(&t, i+1, n, g);
		}
		assert(memsz == 0);
		checkmemo();
		for (i=0; i<MAXPID; ++i) assert(loskiP_findproctab(&t, i) == NULL);
	}

	assert(memsz == 0);
	checkmemo();

	return 0;
}
