#ifndef luaosi_procmgr_h
#define luaosi_procmgr_h


#include "luaosi/proclib.h"

int losiP_initprocmgr (losi_Alloc allocf, void *allocud);

void losiP_lockprocmgr ();

void losiP_unlockprocmgr ();

int losiP_incprocmgr ();

void losiP_putprocmgr (losi_Process *proc);

void losiP_delprocmgr (losi_Process *proc);


#endif
