#ifndef netlibapi_h
#define netlibapi_h

#include "loskiconf.h"

#include <lua.h> /* to copy error messages to Lua */

/*
 * Library
 */

#ifndef LOSKI_DISABLE_NETSTATE
LOSKIDRV_API int loski_opennetwork(loski_NetState *state);

LOSKIDRV_API int loski_closenetwork(loski_NetState *state);
#endif

/*
 * Addresses
 */

enum loski_AddressType { LOSKI_ADDRTYPE_IPV4, LOSKI_ADDRTYPE_IPV6 };

typedef enum loski_AddressType loski_AddressType;

#define LOSKI_ADDRSIZE_IPV4 (4*sizeof(char))
#define LOSKI_ADDRSIZE_IPV6 (16*sizeof(char))

#define LOSKI_ADDRMAXPORT 65535
#define LOSKI_ADDRMAXLITERAL  40  /* 8 groups of 4 digits + 7 ':' + '\0' */


LOSKIDRV_API int loski_initaddress(loski_NetState *state,
                                   loski_Address *address,
                                   loski_AddressType type);


LOSKIDRV_API void loski_setaddrport(loski_NetState *state,
                                    loski_Address *address,
                                    loski_AddressPort port);

LOSKIDRV_API void loski_setaddrbytes(loski_NetState *state,
                                     loski_Address *address,
                                     const char *data);

LOSKIDRV_API int loski_setaddrliteral(loski_NetState *state,
                                      loski_Address *address,
                                      const char *data,
                                      size_t sz);


LOSKIDRV_API loski_AddressType loski_getaddrtype(loski_NetState *state,
                                                 loski_Address *address);

LOSKIDRV_API loski_AddressPort loski_getaddrport(loski_NetState *state,
                                                 loski_Address *address);

LOSKIDRV_API const char *loski_getaddrbytes(loski_NetState *state,
                                            loski_Address *address,
                                            size_t *sz);

LOSKIDRV_API const char *loski_getaddrliteral(loski_NetState *state,
                                              loski_Address *address,
                                              char *data);

/*
 * Sockets
 */

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
