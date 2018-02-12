PROJNAME = losi
LIBNAME = event

NO_DEPEND= YES
USE_LUA52= YES

DEF_FILE= ../etc/tecmake/losi.def
DEFINES= LUA_BUILD_AS_DLL LUA_LIB
INCLUDES= . win32 posix
#LDIR= $(TARGETDIR)
SRC= \
	win32/eventlib.c \
	leventlib.c \
	losiaux.c
SRC+= \
	win32/timeaux.c \
	win32/netlib.c \
	lnetlib.c
#LIBS = time network
LIBS+= wsock32
