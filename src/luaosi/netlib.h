#ifndef luaosi_netlib_h
#define luaosi_netlib_h


#include "luaosi/netdef.h"

#include "luaosi/config.h"
#include "luaosi/errors.h"

/*
 * Library
 */

#ifndef LOSI_DISABLE_NETDRV
LOSIDRV_API losi_ErrorCode losiN_initdrv (losi_NetDriver *drv);

LOSIDRV_API void losiN_freedrv (losi_NetDriver *drv);
#endif

/*
 * Addresses
 */

#ifndef LOSI_ADDRTYPE_CUSTOM
typedef enum losi_AddressType {
	LOSI_ADDRTYPE_IPV4,
	LOSI_ADDRTYPE_IPV6
} losi_AddressType;
#endif

#ifndef LOSI_ADDRSIZE_IPV4
#define LOSI_ADDRSIZE_IPV4 (4*sizeof(char))
#endif

#ifndef LOSI_ADDRSIZE_IPV6
#define LOSI_ADDRSIZE_IPV6 (16*sizeof(char))
#endif

#ifndef LOSI_ADDRMAXPORT
#define LOSI_ADDRMAXPORT 65535
#endif

#ifndef LOSI_ADDRMAXLITERAL
#define LOSI_ADDRMAXLITERAL  47
#endif


LOSIDRV_API losi_ErrorCode losiN_initaddr (losi_NetDriver *drv,
                                           losi_Address *address,
                                           losi_AddressType type);


LOSIDRV_API losi_ErrorCode losiN_setaddrport (losi_NetDriver *drv,
                                              losi_Address *address,
                                              losi_AddressPort port);

LOSIDRV_API losi_ErrorCode losiN_setaddrbytes (losi_NetDriver *drv,
                                               losi_Address *address,
                                               const char *data);

LOSIDRV_API losi_ErrorCode losiN_setaddrliteral (losi_NetDriver *drv,
                                                 losi_Address *address,
                                                 const char *data);


LOSIDRV_API losi_AddressType losiN_getaddrtype (losi_NetDriver *drv,
                                                losi_Address *address);

LOSIDRV_API losi_AddressPort losiN_getaddrport (losi_NetDriver *drv,
                                                losi_Address *address);

LOSIDRV_API const char *losiN_getaddrbytes (losi_NetDriver *drv,
                                            losi_Address *address,
                                            size_t *sz);

LOSIDRV_API const char *losiN_getaddrliteral (losi_NetDriver *drv,
                                              losi_Address *address,
                                              char *data);

/*
 * Sockets
 */

typedef enum losi_SocketType {
	LOSI_SOCKTYPE_LSTN,
	LOSI_SOCKTYPE_STRM,
	LOSI_SOCKTYPE_DGRM
} losi_SocketType;

#ifndef LOSI_SOCKWAY_CUSTOM
typedef enum losi_SocketWay {
	LOSI_SOCKWAY_IN,
	LOSI_SOCKWAY_OUT,
	LOSI_SOCKWAY_BOTH
} losi_SocketWay;
#endif

#ifndef LOSI_SOCKOPT_CUSTOM
typedef enum losi_SocketOption {
	/* BASESOCKET */
	LOSI_SOCKOPT_BLOCKING,
	LOSI_SOCKOPT_REUSEADDR,
	LOSI_SOCKOPT_DONTROUTE,
	/* CONNSOCKET */
	LOSI_SOCKOPT_LINGER,
	LOSI_SOCKOPT_KEEPALIVE,
	LOSI_SOCKOPT_TCPNDELAY,
	/* DRGMSOCKET */
	LOSI_SOCKOPT_BROADCAST
} losi_SocketOption;
#endif

#ifndef LOSI_SOCKRCV_CUSTOM
typedef enum losi_SocketRecvFlag {
	LOSI_SOCKRCV_PEEKONLY = 0x01,
	LOSI_SOCKRCV_WAITALL = 0x02
} losi_SocketRecvFlag;
#endif


LOSIDRV_API losi_ErrorCode losiN_initsock (losi_NetDriver *drv,
                                           losi_Socket *socket,
                                           losi_SocketType type,
                                           losi_AddressType domain);

LOSIDRV_API losi_ErrorCode losiN_getsockid (losi_NetDriver *drv,
                                            losi_Socket *socket);

LOSIDRV_API losi_ErrorCode losiN_setsockopt (losi_NetDriver *drv,
                                             losi_Socket *socket,
                                             losi_SocketOption option,
                                             int value);

LOSIDRV_API losi_ErrorCode losiN_getsockopt (losi_NetDriver *drv,
                                             losi_Socket *socket,
                                             losi_SocketOption option,
                                             int *value);

LOSIDRV_API losi_ErrorCode losiN_bindsock (losi_NetDriver *drv,
                                           losi_Socket *socket,
                                           const losi_Address *address);

LOSIDRV_API losi_ErrorCode losiN_getsockaddr (losi_NetDriver *drv,
                                              losi_Socket *socket,
                                              losi_Address *address,
                                              int peer);

LOSIDRV_API losi_ErrorCode losiN_connectsock (losi_NetDriver *drv,
                                              losi_Socket *socket,
                                              const losi_Address *address);

LOSIDRV_API losi_ErrorCode losiN_sendtosock (losi_NetDriver *drv,
                                             losi_Socket *socket,
                                             const char *data,
                                             size_t size,
                                             size_t *bytes,
                                             const losi_Address *address);

LOSIDRV_API losi_ErrorCode losiN_recvfromsock (losi_NetDriver *drv,
                                               losi_Socket *socket,
                                               losi_SocketRecvFlag flags,
                                               char *buffer,
                                               size_t size,
                                               size_t *bytes,
                                               losi_Address *address);

LOSIDRV_API losi_ErrorCode losiN_shutdownsock (losi_NetDriver *drv,
                                               losi_Socket *socket,
                                               losi_SocketWay way);

LOSIDRV_API losi_ErrorCode losiN_acceptsock (losi_NetDriver *drv,
                                             losi_Socket *socket,
                                             losi_Socket *accepted,
                                             losi_Address *address);

LOSIDRV_API losi_ErrorCode losiN_listensock (losi_NetDriver *drv,
                                             losi_Socket *socket,
                                             int backlog);

LOSIDRV_API losi_ErrorCode losiN_closesock (losi_NetDriver *drv,
                                            losi_Socket *socket);

/*
 * Names
 */

#ifndef LOKSI_ADDRFIND_CUSTOM
typedef enum losi_AddressFindFlag {
	LOSI_ADDRFIND_IPV4 = 0x01,
	LOSI_ADDRFIND_IPV6 = 0x02,
	LOSI_ADDRFIND_MAP4 = 0x04,
	LOSI_ADDRFIND_LSTN = 0x08,
	LOSI_ADDRFIND_DGRM = 0x10,
	LOSI_ADDRFIND_STRM = 0x20
} losi_AddressFindFlag;
#endif

#ifndef LOKSI_ADDRNAME_CUSTOM
typedef enum losi_AddressNameFlag {
	LOSI_ADDRNAME_LOCAL = 0x01,
	LOSI_ADDRNAME_DGRM = 0x02,
} losi_AddressNameFlag;
#endif

#ifndef LOSI_NETNAMEBUFSZ
#define LOSI_NETNAMEBUFSZ	128
#endif

LOSIDRV_API losi_ErrorCode losiN_resolveaddr (losi_NetDriver *drv,
                                              losi_AddressFound *found,
                                              losi_AddressFindFlag flags,
                                              const char *nodename,
                                              const char *servname);

LOSIDRV_API losi_ErrorCode losiN_getaddrfound (losi_NetDriver *drv,
                                               losi_AddressFound *found,
                                               losi_Address *address,
                                               losi_SocketType *type);

LOSIDRV_API void losiN_freeaddrfound (losi_NetDriver *drv,
                                      losi_AddressFound *found);

LOSIDRV_API losi_ErrorCode losiN_getcanonical(losi_NetDriver *drv,
                                              const char *name,
                                              char *buf, size_t sz);

LOSIDRV_API losi_ErrorCode losiN_getaddrnames(losi_NetDriver *drv,
                                              losi_Address *addr,
                                              losi_AddressNameFlag flags,
                                              char *nodebuf, size_t nodesz,
                                              char *servbuf, size_t servsz);

#endif
