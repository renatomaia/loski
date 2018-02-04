# Makefile for building Lua
# See ../doc/readme.html for installation and customization instructions.

include ../config

# == CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT =======================

PLAT_DIR= posix
CC= gcc
LD= ld
CFLAGS= -O2 -Wall -I../src -I../src/$(PLAT_DIR) -I$(LUA_INC) $(SYSCFLAGS) $(MYCFLAGS)
LDFLAGS= $(SYSLDFLAGS) $(MYLDFLAGS)
LIBS= $(SYSLIBS) $(MYLIBS)

SYSCFLAGS=
SYSLDFLAGS=
SYSLIBS=

MYCFLAGS=
MYLDFLAGS=
MYLIBS=
MYOBJS=

# == END OF USER SETTINGS -- NO NEED TO CHANGE ANYTHING BELOW THIS LINE =======

TAB_O=	../src/$(PLAT_DIR)/proctab.o
TST_O=	testproctab.o

TST_T=	testproctab

ALL_O=	$(TST_O) $(TAB_O)
ALL_T=	$(TST_T)

# Targets start here.
default: $(PLAT)

all: $(TST_T)
	./$(TST_T)

$(TST_T):	$(TST_O) $(TAB_O)
	$(CC) -o $@ $(LDFLAGS) $(TST_O) $(TAB_O)

clean:
	$(RM) $(ALL_T) $(ALL_O)

depend:
	@$(CC) $(CFLAGS) -MM l*.c

echo:
	@echo "PLAT= $(PLAT)"
	@echo "CC= $(CC)"
	@echo "CFLAGS= $(CFLAGS)"
	@echo "LDFLAGS= $(LDFLAGS)"
	@echo "LIBS= $(LIBS)"
	@echo "AR= $(AR)"
	@echo "RANLIB= $(RANLIB)"
	@echo "RM= $(RM)"

# Convenience targets for popular platforms
ALL= all

none:
	@echo "Please do 'make PLATFORM' where PLATFORM is one of these:"
	@echo "   $(PLATS)"

generic: $(ALL)

linux:
	$(MAKE) $(ALL) #SYSCFLAGS="-fpic" SYSLDFLAGS="-shared"

macosx:
	$(MAKE) $(ALL) CC='export MACOSX_DEPLOYMENT_TARGET="10.3"; gcc' \
	               LD='export MACOSX_DEPLOYMENT_TARGET="10.3"; gcc'

solaris:
	$(MAKE) $(ALL) SYSCFLAGS="-fpic" SYSLDFLAGS="-O -shared"

# list targets that do not create files (but not all makes understand .PHONY)
.PHONY: all $(PLATS) default o a so clean depend echo none

# DO NOT DELETE
