#include "loskiconf.h"
#include "netlib.h"

#include <lua.h>


#define LOSKI_STRMSOCKET 3
#define LOSKI_BASESOCKET 4

static const char *const loski_SocketClasses[] = {
	LOSKI_PREFIX"network.DatagramSocket",
	LOSKI_PREFIX"network.ConnectionSocket",
	LOSKI_PREFIX"network.ListenSocket",
	LOSKI_PREFIX"network.StreamSocket"
	LOSKI_PREFIX"network.Socket"
};

LUAMOD_API int luaopen_network (lua_State *L);

LOSKILIB_API int loski_issocket (lua_State *L, int idx, int cls);
LOSKILIB_API loski_Socket *loski_tosocket (lua_State *L, int idx, int cls);
