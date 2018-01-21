#ifndef luaosi_netlib_h
#define luaosi_netlib_h


#include "luaosi/netdef.h"

#include "luaosi/config.h"
#include "luaosi/errors.h"

/*
 * Library
 */

#ifndef LOSKI_DISABLE_NETDRV
LOSKIDRV_API loski_ErrorCode loskiN_initdrv (loski_NetDriver *drv);

LOSKIDRV_API void loskiN_freedrv (loski_NetDriver *drv);
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
#define LOSKI_ADDRMAXLITERAL  47
#endif


LOSKIDRV_API loski_ErrorCode loskiN_initaddr (loski_NetDriver *drv,
                                              loski_Address *address,
                                              loski_AddressType type);


LOSKIDRV_API loski_ErrorCode loskiN_setaddrport (loski_NetDriver *drv,
                                                 loski_Address *address,
                                                 loski_AddressPort port);

LOSKIDRV_API loski_ErrorCode loskiN_setaddrbytes (loski_NetDriver *drv,
                                                  loski_Address *address,
                                                  const char *data);

LOSKIDRV_API loski_ErrorCode loskiN_setaddrliteral (loski_NetDriver *drv,
                                                    loski_Address *address,
                                                    const char *data);


LOSKIDRV_API loski_AddressType loskiN_getaddrtype (loski_NetDriver *drv,
                                                   loski_Address *address);

LOSKIDRV_API loski_AddressPort loskiN_getaddrport (loski_NetDriver *drv,
                                                   loski_Address *address);

LOSKIDRV_API const char *loskiN_getaddrbytes (loski_NetDriver *drv,
                                              loski_Address *address,
                                              size_t *sz);

LOSKIDRV_API const char *loskiN_getaddrliteral (loski_NetDriver *drv,
                                                loski_Address *address,
                                                char *data);

/*
 * Sockets
 */

typedef enum loski_SocketType {
	LOSKI_SOCKTYPE_LSTN,
	LOSKI_SOCKTYPE_CONN,
	LOSKI_SOCKTYPE_DGRM
} loski_SocketType;

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
	LOSKI_SOCKRCV_PEEKONLY = 0x01,
	LOSKI_SOCKRCV_WAITALL = 0x02
} loski_SocketRecvFlag;
#endif


LOSKIDRV_API loski_ErrorCode loskiN_initsock (loski_NetDriver *drv,
                                              loski_Socket *socket,
                                              loski_SocketType type,
                                              loski_AddressType domain);

LOSKIDRV_API loski_ErrorCode loskiN_getsockid (loski_NetDriver *drv,
                                               loski_Socket *socket);

LOSKIDRV_API loski_ErrorCode loskiN_setsockopt (loski_NetDriver *drv,
                                                loski_Socket *socket,
                                                loski_SocketOption option,
                                                int value);

LOSKIDRV_API loski_ErrorCode loskiN_getsockopt (loski_NetDriver *drv,
                                                loski_Socket *socket,
                                                loski_SocketOption option,
                                                int *value);

LOSKIDRV_API loski_ErrorCode loskiN_bindsock (loski_NetDriver *drv,
                                              loski_Socket *socket,
                                              const loski_Address *address);

LOSKIDRV_API loski_ErrorCode loskiN_getsockaddr (loski_NetDriver *drv,
                                                 loski_Socket *socket,
                                                 loski_Address *address,
                                                 int peer);

LOSKIDRV_API loski_ErrorCode loskiN_connectsock (loski_NetDriver *drv,
                                                 loski_Socket *socket,
                                                 const loski_Address *address);

LOSKIDRV_API loski_ErrorCode loskiN_sendtosock (loski_NetDriver *drv,
                                                loski_Socket *socket,
                                                const char *data,
                                                size_t size,
                                                size_t *bytes,
                                                const loski_Address *address);

LOSKIDRV_API loski_ErrorCode loskiN_recvfromsock (loski_NetDriver *drv,
                                                  loski_Socket *socket,
                                                  loski_SocketRecvFlag flags,
                                                  char *buffer,
                                                  size_t size,
                                                  size_t *bytes,
                                                  loski_Address *address);

LOSKIDRV_API loski_ErrorCode loskiN_shutdownsock (loski_NetDriver *drv,
                                                  loski_Socket *socket,
                                                  loski_SocketWay way);

LOSKIDRV_API loski_ErrorCode loskiN_acceptsock (loski_NetDriver *drv,
                                                loski_Socket *socket,
                                                loski_Socket *accepted,
                                                loski_Address *address);

LOSKIDRV_API loski_ErrorCode loskiN_listensock (loski_NetDriver *drv,
                                                loski_Socket *socket,
                                                int backlog);

LOSKIDRV_API loski_ErrorCode loskiN_closesock (loski_NetDriver *drv,
                                               loski_Socket *socket);

/*
 * Names
 */

#ifndef LOKSI_ADDRFIND_CUSTOM
typedef enum loski_AddressFindFlag {
	LOSKI_ADDRFIND_IPV4 = 0x01,
	LOSKI_ADDRFIND_IPV6 = 0x02,
	LOSKI_ADDRFIND_MAP4 = 0x04,
	LOSKI_ADDRFIND_LSTN = 0x08,
	LOSKI_ADDRFIND_DGRM = 0x10,
	LOSKI_ADDRFIND_STRM = 0x20
} loski_AddressFindFlag;
#endif

#ifndef LOKSI_ADDRNAME_CUSTOM
typedef enum loski_AddressNameFlag {
	LOSKI_ADDRNAME_LOCAL = 0x01,
	LOSKI_ADDRNAME_DGRM = 0x02,
} loski_AddressNameFlag;
#endif

#ifndef LOSKI_NETNAMEBUFSZ
#define LOSKI_NETNAMEBUFSZ	128
#endif

LOSKIDRV_API loski_ErrorCode loskiN_resolveaddr (loski_NetDriver *drv,
                                                 loski_AddressFound *found,
                                                 loski_AddressFindFlag flags,
                                                 const char *nodename,
                                                 const char *servname);

LOSKIDRV_API loski_ErrorCode loskiN_getaddrfound (loski_NetDriver *drv,
                                                  loski_AddressFound *found,
                                                  loski_Address *address,
                                                  loski_SocketType *type);

LOSKIDRV_API void loskiN_freeaddrfound (loski_NetDriver *drv,
                                        loski_AddressFound *found);

LOSKIDRV_API loski_ErrorCode loskiN_getcanonical(loski_NetDriver *drv,
                                                 const char *name,
                                                 char *buf, size_t sz);

LOSKIDRV_API loski_ErrorCode loskiN_getaddrnames(loski_NetDriver *drv,
                                                 loski_Address *addr,
                                                 loski_AddressNameFlag flags,
                                                 char *nodebuf, size_t nodesz,
                                                 char *servbuf, size_t servsz);

#endif
