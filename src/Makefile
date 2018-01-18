# Makefile for building Lua
# See ../doc/readme.html for installation and customization instructions.

include ../config

# == CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT =======================

PLAT_DIR= posix
CC= gcc
LD= ld
CFLAGS= -O2 -Wall -I. -I$(LUA_INC) -I$(PLAT_DIR) $(SYSCFLAGS) $(MYCFLAGS)
LDFLAGS= $(SYSLDFLAGS) $(MYLDFLAGS)
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

AUX_O=	loskiaux.o
TIM_O=	ltimelib.o $(PLAT_DIR)/timelib.o
PRC_O=	lproclib.o $(PLAT_DIR)/proclib.o $(PLAT_DIR)/st/procmgr.o $(PLAT_DIR)/proctab.o
NET_O=	lnetlib.o $(PLAT_DIR)/netlib.o
EVT_O=	leventlib.o $(PLAT_DIR)/eventlib.o $(PLAT_DIR)/timeaux.o lnetaux.o
#FLS_O=	lfilesys.o $(PLAT_DIR)/filesyslib.o

ALL_O=	$(AUX_O) $(TIM_O) $(PRC_O) $(NET_O) $(EVT_O) $(FLS_O) $(MYOBJS)
ALL_A= $(LIB_A)
ALL_M= $(TIM_M) $(PRC_M) $(NET_M) $(EVT_M) $(FLS_M)
ALL_T= $(ALL_A) $(ALL_M)

# Targets start here.
default: $(PLAT)

all:	$(ALL_T)

o:	$(ALL_O)

a:	$(ALL_A)

m:	$(ALL_M)

$(TIM_M): $(TIM_O) $(AUX_O)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(PRC_M): $(PRC_O) $(AUX_O)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(NET_M): $(NET_O) $(AUX_O)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(EVT_M): $(EVT_O) $(AUX_O)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(FLS_M): $(FLS_O) $(AUX_O)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

$(LIB_A): $(ALL_O)
	$(AR) $@ $?
	$(RANLIB) $@

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
loskiaux.o: loskiaux.c loskiaux.h loskierr.h loskiconf.h
 
leventlib.o: leventlib.c lnetaux.h \
             netlib.h posix/netdef.h \
             proclib.h posix/procdef.h \
             timelib.h posix/timedef.h \
             eventlib.h posix/eventdef.h \
             loskiaux.h loskierr.h loskiconf.h

lnetaux.o: lnetaux.c lnetaux.h \
           netlib.h posix/netdef.h \
           loskiaux.h loskierr.h loskiconf.h
lnetlib.o: lnetlib.c lnetaux.h \
           netlib.h posix/netdef.h \
           loskiaux.h loskierr.h loskiconf.h

lprocaux.o: lprocaux.c lprocaux.h \
            proclib.h posix/procdef.h \
            loskiaux.h loskierr.h loskiconf.h
lproclib.o: lproclib.c lprocaux.h \
            proclib.h posix/procdef.h \
            loskiaux.h loskierr.h loskiconf.h

ltimelib.o: ltimelib.c \
            timelib.h posix/timedef.h \
            loskiaux.h loskierr.h loskiconf.h

luaoski.o: luaoski.c luaoski.h
