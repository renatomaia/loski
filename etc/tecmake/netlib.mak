PROJNAME = losi
LIBNAME = network

NO_DEPEND= YES
USE_LUA52= YES

DEF_FILE= ../etc/tecmake/losi.def
DEFINES= LUA_BUILD_AS_DLL LUA_LIB
INCLUDES= . win32
SRC= \
	win32/netlib.c \
	lnetlib.c \
	losiaux.c
LIBS = wsock32
