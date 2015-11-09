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

#ifndef LOSKI_ADDRTYPE_CUSTOM
typedef enum loski_AddressType {
	LOSKI_ADDRTYPE_IPV4,
	LOSKI_ADDRTYPE_IPV6
} loski_AddressType;
#endif

#ifndef LOSKI_ADDRSIZE_IPV4
#define LOSKI_ADDRSIZE_IPV4 (4*sizeof(char))
#endif

#ifndef LOSKI_ADDRSIZE_IPV6
#define LOSKI_ADDRSIZE_IPV6 (16*sizeof(char))
#endif

#ifndef LOSKI_ADDRMAXPORT
#define LOSKI_ADDRMAXPORT 65535
#endif

#ifndef LOSKI_ADDRMAXLITERAL
#define LOSKI_ADDRMAXLITERAL  40  /* 8 groups of 4 digits + 7 ':' + '\0' */
#endif


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

#ifndef LOSKI_SOCKTYPE_CUSTOM
typedef enum loski_SocketType {
	LOSKI_LSTNSOCKET,
	LOSKI_CONNSOCKET,
	LOSKI_DGRMSOCKET
} loski_SocketType;
#endif

#ifndef LOSKI_SOCKWAY_CUSTOM
typedef enum loski_SocketWay {
	LOSKI_SOCKWAY_IN,
	LOSKI_SOCKWAY_OUT,
	LOSKI_SOCKWAY_BOTH
} loski_SocketWay;
#endif

#ifndef LOSKI_SOCKOPT_CUSTOM
typedef enum loski_SocketOption {
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
} loski_SocketOption;
#endif

#ifndef LOSKI_SOCKRCV_CUSTOM
typedef enum loski_SocketRecvFlag {
	LOSKI_SOCKRCV_PEEKONLY = 1,
	LOSKI_SOCKRCV_WAITALL = 2
} loski_SocketRecvFlag;
#endif


LOSKIDRV_API int loski_createsocket(loski_NetState *state,
                                    loski_Socket *socket,
                                    loski_SocketType type,
                                    loski_AddressType domain);

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
                                     int peer);

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
                                      loski_SocketRecvFlag flags,
                                      char *buffer,
                                      size_t size,
                                      size_t *bytes,
                                      loski_Address *address);

LOSKIDRV_API int loski_shutdownsocket(loski_NetState *state,
                                      loski_Socket *socket,
                                      loski_SocketWay way);

LOSKIDRV_API int loski_acceptsocket(loski_NetState *state,
                                    loski_Socket *socket,
                                    loski_Socket *accepted,
                                    loski_Address *address);

LOSKIDRV_API int loski_listensocket(loski_NetState *state,
                                    loski_Socket *socket,
                                    int backlog);

LOSKIDRV_API int loski_closesocket(loski_NetState *state,
                                   loski_Socket *socket);

/*
 * Names
 */

#ifndef LOKSI_ADDRFIND_CUSTOM
typedef enum loski_AddressFindFlag {
	LOSKI_ADDRFIND_LOCAL = 1,
	LOSKI_ADDRFIND_IPV4 = 2,
	LOSKI_ADDRFIND_IPV6 = 4,
	LOSKI_ADDRFIND_MAPPED = 8,
	LOSKI_ADDRFIND_DGRM = 16,
	LOSKI_ADDRFIND_STRM = 32
} loski_AddressFindFlag;
#endif


LOSKIDRV_API int loski_netresolveaddr(loski_NetState *state,
                                      loski_AddressFound *found,
                                      loski_AddressFindFlag flags,
                                      const char *nodename,
                                      const char *servname);

LOSKIDRV_API int loski_netgetaddrfound(loski_NetState *state,
                                       loski_AddressFound *found,
                                       loski_Address *address,
                                       loski_SocketType *type);

LOSKIDRV_API void loski_netfreeaddrfound(loski_NetState *state,
                                         loski_AddressFound *found);

#endif
