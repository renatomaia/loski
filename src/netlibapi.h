#ifndef netlibapi_h
#define netlibapi_h

#include "loskiconf.h"

#include <lua.h> /* to copy error messages to Lua */

enum loski_SocketSite { LOSKI_LOCALSITE, LOSKI_REMOTESITE, LOSKI_BOTHSITES };
enum loski_SocketType { LOSKI_LSTNSOCKET, LOSKI_CONNSOCKET, LOSKI_DGRMSOCKET };
enum loski_SocketOption {
	/* BASESOCKET */
	LOSKI_SOCKOPT_BLOCKING,
	LOSKI_SOCKOPT_REUSEADDR,
	LOSKI_SOCKOPT_DONTROUTE,
	/* CONNSOCKET */
	LOSKI_SOCKOPT_LINGER,
	LOSKI_SOCKOPT_KEEPALIVE,
	LOSKI_SOCKOPT_TCPNDELAY,
	/* DRGMSOCKET */
	LOSKI_SOCKOPT_BROADCAST
};

typedef enum loski_SocketSite loski_SocketSite;
typedef enum loski_SocketType loski_SocketType;
typedef enum loski_SocketOption loski_SocketOption;


#ifndef LOSKI_DISABLE_NETSTATE
LOSKIDRV_API int loski_opennetwork(loski_NetState *state);

LOSKIDRV_API int loski_closenetwork(loski_NetState *state);
#endif


LOSKIDRV_API int loski_socketerror(int error, lua_State *L);

LOSKIDRV_API int loski_createsocket(loski_NetState *state,
                                    loski_Socket *socket,
                                    loski_SocketType type);

LOSKIDRV_API int loski_getsocketid(loski_NetState *state, loski_Socket *socket);

LOSKIDRV_API int loski_setsocketoption(loski_NetState *state,
                                       loski_Socket *socket,
                                       loski_SocketOption option,
                                       int value);

LOSKIDRV_API int loski_getsocketoption(loski_NetState *state,
                                       loski_Socket *socket,
                                       loski_SocketOption option,
                                       int *value);

LOSKIDRV_API int loski_bindsocket(loski_NetState *state,
                                  loski_Socket *socket,
                                  const loski_Address *address);

LOSKIDRV_API int loski_socketaddress(loski_NetState *state,
                                     loski_Socket *socket,
                                     loski_Address *address,
                                     loski_SocketSite site);

LOSKIDRV_API int loski_connectsocket(loski_NetState *state,
                                     loski_Socket *socket,
                                     const loski_Address *address);

LOSKIDRV_API int loski_sendtosocket(loski_NetState *state,
                                    loski_Socket *socket,
                                    const char *data,
                                    size_t size,
                                    size_t *bytes,
                                    const loski_Address *address);

LOSKIDRV_API int loski_recvfromsocket(loski_NetState *state,
                                      loski_Socket *socket,
                                      char *buffer,
                                      size_t size,
                                      size_t *bytes,
                                      loski_Address *address);

LOSKIDRV_API int loski_shutdownsocket(loski_NetState *state,
                                      loski_Socket *socket,
                                      loski_SocketSite site);

LOSKIDRV_API int loski_acceptsocket(loski_NetState *state,
                                    loski_Socket *socket,
                                    loski_Socket *accepted,
                                    loski_Address *address);

LOSKIDRV_API int loski_listensocket(loski_NetState *state,
                                    loski_Socket *socket,
                                    int backlog);

LOSKIDRV_API int loski_closesocket(loski_NetState *state,
                                   loski_Socket *socket);

#endif
