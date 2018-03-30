# Makefile for installing LOSI
# See README.md for installation and customization instructions.

include ../config

# == CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT =======================

PLAT_DIR= posix
CC= gcc -std=gnu99
LD= ld
CFLAGS= -O2 -Wall -I. -I$(LUA_INC) -I$(LUAMEM_INC) -I$(PLAT_DIR) \
        $(SYSCFLAGS) $(MYCFLAGS)
LDFLAGS= -L$(LUAMEM_LIB) $(SYSLDFLAGS) $(MYLDFLAGS)
LIBS= $(SYSLIBS) $(MYLIBS)

AR= ar rcu
RANLIB= ranlib
RM= rm -f

SYSCFLAGS=
SYSLDFLAGS=
SYSLIBS=

MYCFLAGS=
MYLDFLAGS=
MYLIBS=
MYOBJS=

# == END OF USER SETTINGS -- NO NEED TO CHANGE ANYTHING BELOW THIS LINE =======

RTL_O=	lauxlib.o \
      	$(PLAT_DIR)/fdlib.o \
      	$(PLAT_DIR)/timelib.o $(PLAT_DIR)/ltimelib.o \
      	$(PLAT_DIR)/proclib.o lproclib.o \
      	$(PLAT_DIR)/st/procmgr.o $(PLAT_DIR)/proctab.o \
      	$(PLAT_DIR)/filelib.o lfilelib.o \
      	$(PLAT_DIR)/netlib.o lnetlib.o \
      	$(PLAT_DIR)/evtlib.o levtlib.o
LIB_O=	luaosi.o
TIM_O=	ltimemod.o
PRC_O=	lprocmod.o
FIL_O=	lfilemod.o
NET_O=	lnetmod.o
EVT_O=	levtmod.o

ALL_O=	$(RTL_O) $(LIB_O) $(TIM_O) $(PRC_O) $(FIL_O) $(NET_O) $(EVT_O) \
      	$(MYOBJS)
ALL_A=	$(LIB_A)
ALL_S=	$(LIB_S) $(RTL_S)
ALL_M=	$(TIM_M) $(PRC_M) $(FIL_M) $(NET_M) $(EVT_M)
ALL_T=	$(ALL_A) $(ALL_S) $(ALL_M)

# Targets start here.
default: $(PLAT)

all:	$(ALL_T)

o:	$(ALL_O)

a:	$(ALL_A)

so:	$(ALL_S)

m:	$(ALL_M)

$(RTL_S): $(RTL_O)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(TIM_M): $(TIM_O) $(RTL_S)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(PRC_M): $(PRC_O) $(RTL_S)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(FIL_M): $(FIL_O) $(RTL_S)
	$(LD) -o $@ -lluamemlib $(LDFLAGS) $^ $(LIBS)

$(NET_M): $(NET_O) $(RTL_S)
	$(LD) -o $@ -lluamemlib $(LDFLAGS) $^ $(LIBS)

$(EVT_M): $(EVT_O) $(RTL_S)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(LIB_S): $(LIB_O) $(RTL_S) $(ALL_M)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(LIB_A): $(ALL_O)
	$(AR) $@ $?
	$(RANLIB) $@

clean:
	$(RM) $(ALL_T) $(ALL_O)

depend:
	@$(CC) $(CFLAGS) -MM *.c $(PLAT_DIR)/*.c $(PLAT_DIR)/st/*.c

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
	$(MAKE) $(ALL) SYSCFLAGS="-fpic" SYSLDFLAGS="-shared"

macosx:
	$(MAKE) $(ALL) SYSCFLAGS="-fno-common" \
	               SYSLDFLAGS="-bundle -undefined dynamic_lookup" \
	               CC='export MACOSX_DEPLOYMENT_TARGET="10.3"; gcc' \
	               LD='export MACOSX_DEPLOYMENT_TARGET="10.3"; gcc'

solaris:
	$(MAKE) $(ALL) SYSCFLAGS="-fpic" SYSLDFLAGS="-O -shared"

# list targets that do not create files (but not all makes understand .PHONY)
.PHONY: all $(PLATS) default o a so clean depend echo none

# DO NOT DELETE
lauxlib.o:              lauxlib.c luaosi/lauxlib.h \
                        luaosi/errors.h luaosi/config.h
$(PLAT_DIR)/timelib.o:  $(PLAT_DIR)/timelib.c luaosi/timelib.h $(PLAT_DIR)/luaosi/timedef.h \
                        luaosi/config.h
$(PLAT_DIR)/proctab.o:  $(PLAT_DIR)/proctab.c $(PLAT_DIR)/luaosi/proctab.h \
                        luaosi/proclib.h $(PLAT_DIR)/luaosi/procdef.h \
                        luaosi/errors.h luaosi/config.h
$(PLAT_DIR)/procmgr.o:  posix/st/procmgr.c posix/luaosi/procmgr.h posix/luaosi/proctab.h \
                        luaosi/proclib.h posix/luaosi/procdef.h \
                        luaosi/config.h luaosi/errors.h
$(PLAT_DIR)/proclib.o:  $(PLAT_DIR)/proclib.c luaosi/proclib.h \
                        $(PLAT_DIR)/luaosi/procdef.h \
                        luaosi/errors.h luaosi/config.h \
                        $(PLAT_DIR)/luaosi/procmgr.h
$(PLAT_DIR)/netlib.o:   $(PLAT_DIR)/netlib.c luaosi/netlib.h $(PLAT_DIR)/luaosi/netdef.h \
                        luaosi/evtlib.h $(PLAT_DIR)/luaosi/evtdef.h \
                        luaosi/lauxlib.h luaosi/errors.h luaosi/config.h
$(PLAT_DIR)/evtlib.o:   $(PLAT_DIR)/evtlib.c luaosi/evtlib.h \
                        $(PLAT_DIR)/luaosi/evtdef.h \
                        luaosi/levtlib.h \
                        $(PLAT_DIR)/luaosi/ltimelib.h \
                        luaosi/lauxlib.h luaosi/errors.h luaosi/config.h
$(PLAT_DIR)/ltimelib.o: $(PLAT_DIR)/ltimelib.c $(PLAT_DIR)/luaosi/ltimelib.h \
                        luaosi/config.h
lnetlib.o:              lnetlib.c luaosi/lnetlib.h \
                        luaosi/netlib.h $(PLAT_DIR)/luaosi/netdef.h \
                        luaosi/evtlib.h $(PLAT_DIR)/luaosi/evtdef.h \
                        luaosi/lauxlib.h luaosi/errors.h luaosi/config.h
levtlib.o:              levtlib.c luaosi/levtlib.h \
                        luaosi/evtlib.h $(PLAT_DIR)/luaosi/evtdef.h \
                        luaosi/lauxlib.h luaosi/errors.h luaosi/config.h
ltimemod.o:             ltimemod.c \
                        luaosi/timelib.h $(PLAT_DIR)/luaosi/timedef.h \
                        luaosi/lauxlib.h luaosi/errors.h luaosi/config.h
lprocmod.o:             lprocmod.c \
                        luaosi/lproclib.h luaosi/proclib.h $(PLAT_DIR)/luaosi/procdef.h \
                        luaosi/lauxlib.h luaosi/errors.h luaosi/config.h
lnetmod.o:              lnetmod.c \
                        luaosi/lnetlib.h luaosi/netlib.h $(PLAT_DIR)/luaosi/netdef.h \
                        luaosi/levtlib.h luaosi/evtlib.h $(PLAT_DIR)/luaosi/evtdef.h \
                        luaosi/lauxlib.h luaosi/errors.h luaosi/config.h
levtmod.o:              levtmod.c \
                        luaosi/levtlib.h luaosi/evtlib.h $(PLAT_DIR)/luaosi/evtdef.h \
                        luaosi/lauxlib.h luaosi/errors.h luaosi/config.h
luaosi.o:               luaosi.c luaosi.h
