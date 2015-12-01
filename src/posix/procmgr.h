#ifndef procmgr_h
#define procmgr_h


#include "proclib.h"

int loskiP_initprocmgr (loski_Alloc allocf, void *allocud);

void loskiP_lockprocmgr ();

void loskiP_unlockprocmgr ();

int loskiP_incprocmgr ();

void loskiP_putprocmgr (loski_Process *proc);

void loskiP_delprocmgr (loski_Process *proc);


#endif
