#include "loskiconf.h"
#include "addrlib.h"

#include <lua.h>


#define LOSKI_NETADDRCLS LOSKI_PREFIX"network.Address"


LUAMOD_API int luaopen_network_address (lua_State *L);

LOSKILIB_API int loski_isaddress (lua_State *L, int idx);
LOSKILIB_API loski_Address *loski_toaddress (lua_State *L, int idx);
