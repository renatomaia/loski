#ifndef netlibapi_h
#define netlibapi_h

#include "loskiconf.h"

/*
 * Library
 */

#ifndef LOSKI_DISABLE_NETDRV
LOSKIDRV_API int loski_opennetwork(loski_NetDriver *state);

LOSKIDRV_API int loski_closenetwork(loski_NetDriver *state);
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


LOSKIDRV_API int loskiN_initaddr(loski_NetDriver *state,
                                 loski_Address *address,
                                 loski_AddressType type);


LOSKIDRV_API void loskiN_setaddrport(loski_NetDriver *state,
                                     loski_Address *address,
                                     loski_AddressPort port);

LOSKIDRV_API void loskiN_setaddrbytes(loski_NetDriver *state,
                                      loski_Address *address,
                                      const char *data);

LOSKIDRV_API int loskiN_setaddrliteral(loski_NetDriver *state,
                                       loski_Address *address,
                                       const char *data,
                                       size_t sz);


LOSKIDRV_API loski_AddressType loskiN_getaddrtype(loski_NetDriver *state,
                                                  loski_Address *address);

LOSKIDRV_API loski_AddressPort loskiN_getaddrport(loski_NetDriver *state,
                                                  loski_Address *address);

LOSKIDRV_API const char *loskiN_getaddrbytes(loski_NetDriver *state,
                                             loski_Address *address,
                                             size_t *sz);

LOSKIDRV_API const char *loskiN_getaddrliteral(loski_NetDriver *state,
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


LOSKIDRV_API int loskiN_initsock(loski_NetDriver *state,
                                   loski_Socket *socket,
                                   loski_SocketType type,
                                   loski_AddressType domain);

LOSKIDRV_API int loskiN_getsockid(loski_NetDriver *state, loski_Socket *socket);

LOSKIDRV_API int loskiN_setsockopt(loski_NetDriver *state,
                                   loski_Socket *socket,
                                   loski_SocketOption option,
                                   int value);

LOSKIDRV_API int loskiN_getsockopt(loski_NetDriver *state,
                                   loski_Socket *socket,
                                   loski_SocketOption option,
                                   int *value);

LOSKIDRV_API int loskiN_bindsock(loski_NetDriver *state,
                                 loski_Socket *socket,
                                 const loski_Address *address);

LOSKIDRV_API int loskiN_getsockaddr(loski_NetDriver *state,
                                   loski_Socket *socket,
                                   loski_Address *address,
                                   int peer);

LOSKIDRV_API int loskiN_connectsock(loski_NetDriver *state,
                                    loski_Socket *socket,
                                    const loski_Address *address);

LOSKIDRV_API int loskiN_sendtosock(loski_NetDriver *state,
                                   loski_Socket *socket,
                                   const char *data,
                                   size_t size,
                                   size_t *bytes,
                                   const loski_Address *address);

LOSKIDRV_API int loskiN_recvfromsock(loski_NetDriver *state,
                                     loski_Socket *socket,
                                     loski_SocketRecvFlag flags,
                                     char *buffer,
                                     size_t size,
                                     size_t *bytes,
                                     loski_Address *address);

LOSKIDRV_API int loskiN_shutdownsock(loski_NetDriver *state,
                                     loski_Socket *socket,
                                     loski_SocketWay way);

LOSKIDRV_API int loskiN_acceptsock(loski_NetDriver *state,
                                   loski_Socket *socket,
                                   loski_Socket *accepted,
                                   loski_Address *address);

LOSKIDRV_API int loskiN_listensock(loski_NetDriver *state,
                                   loski_Socket *socket,
                                   int backlog);

LOSKIDRV_API int loskiN_closesock(loski_NetDriver *state,
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


LOSKIDRV_API int loskiN_resolveaddr(loski_NetDriver *state,
                                    loski_AddressFound *found,
                                    loski_AddressFindFlag flags,
                                    const char *nodename,
                                    const char *servname);

LOSKIDRV_API int loskiN_getaddrfound(loski_NetDriver *state,
                                     loski_AddressFound *found,
                                     loski_Address *address,
                                     loski_SocketType *type);

LOSKIDRV_API void loskiN_freeaddrfound(loski_NetDriver *state,
                                       loski_AddressFound *found);

#endif
