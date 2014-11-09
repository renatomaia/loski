PROJNAME = loski
LIBNAME = event

NO_DEPEND= YES
USE_LUA52= YES

DEF_FILE= ../etc/tecmake/loski.def
DEFINES= LUA_BUILD_AS_DLL LUA_LIB
INCLUDES= . win32 posix
#LDIR= $(TARGETDIR)
SRC= \
	win32/eventlib.c \
	leventlib.c \
	loskiaux.c
SRC+= \
	win32/timeaux.c \
	win32/netlib.c \
	lnetlib.c
#LIBS = time network
LIBS+= wsock32
