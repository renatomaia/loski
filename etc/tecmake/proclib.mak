PROJNAME = loski
LIBNAME = process

NO_DEPEND= YES
USE_LUA52= YES

DEF_FILE= ../etc/tecmake/loski.def
DEFINES= LUA_BUILD_AS_DLL LUA_LIB
INCLUDES= . win32
SRC= \
	win32/proclib.c \
	lproclib.c \
	loskiaux.c
