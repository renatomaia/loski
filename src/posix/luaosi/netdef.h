#ifndef luaosi_netdef_h
#define luaosi_netdef_h


typedef void losi_NetDriver;


/* Addresses */

#include <netinet/in.h>  /* network addresses */
#include <arpa/inet.h>  /* IP addresses */
#include <sys/un.h>  /* UNIX domain addresses */

typedef struct sockaddr losi_Address;

typedef in_port_t losi_AddressPort;

#define LOSI_ADDRSIZE_IPV4	sizeof(struct sockaddr_in)
#define LOSI_ADDRSIZE_IPV6	sizeof(struct sockaddr_in6)
#define LOSI_ADDRSIZE_FILE	sizeof(struct sockaddr_un)

typedef int losi_AddressType;
#define LOSI_ADDRTYPE_IPV4	AF_INET
#define LOSI_ADDRTYPE_IPV6	AF_INET6
#define LOSI_ADDRTYPE_FILE	AF_UNIX
#define LOSI_ADDRTYPE_CUSTOM

#define LOSI_ADDRMAXLITERAL  (INET6_ADDRSTRLEN+1)


/* Sockets */

#include "luaosi/evtlib.h"

#include <sys/socket.h>

typedef struct losi_Socket {
	int fd;
	int domain;
} losi_Socket;

typedef int losi_SocketRecvFlag;
#define LOSI_SOCKRCV_PEEKONLY	MSG_PEEK
#define LOSI_SOCKRCV_WAITALL	MSG_WAITALL
#define LOSI_SOCKRCV_CUSTOM

typedef int losi_SocketWay;
#define LOSI_SOCKWAY_IN	SHUT_RD;
#define LOSI_SOCKWAY_OUT	SHUT_WR;
#define LOSI_SOCKWAY_BOTH	SHUT_RDWR;
#define LOSI_SOCKWAY_CUSTOM

#define LOSI_ENABLE_SOCKETEVENTS

LOSIDRV_API losi_ErrorCode losiN_getsockevtsrc (void *udata, int newref,
                                                losi_EventSource *src,
                                                losi_EventFlags evtflags);
LOSIDRV_API void losiN_freesockevtsrc (void *udata);


/* Names */

#include <netdb.h>

typedef struct losi_AddressFound {
	struct addrinfo *results;
	struct addrinfo *next;
	int passive;
} losi_AddressFound;

typedef int losi_AddressNameFlag;
#define LOSI_ADDRNAME_LOCAL	NI_NOFQDN
#define LOSI_ADDRNAME_DGRM	NI_DGRAM
#define LOSI_ADDRNAME_CUSTOM



#endif
