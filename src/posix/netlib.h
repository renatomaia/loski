#ifndef netlib_h
#define netlib_h


#include <loskierr.h>


/* Addresses */

#include <netinet/in.h>  /* network addresses */
#include <arpa/inet.h>  /* IP addresses */

typedef void loski_NetDriver;

typedef struct sockaddr_storage loski_Address;

typedef in_port_t loski_AddressPort;

typedef int loski_AddressType;
#define LOSKI_ADDRTYPE_IPV4	AF_INET
#define LOSKI_ADDRTYPE_IPV6	AF_INET6
#define LOSKI_ADDRTYPE_CUSTOM

#define LOSKI_ADDRMAXLITERAL  (INET6_ADDRSTRLEN+1)


/* Sockets */

#include <sys/socket.h>

typedef int loski_Socket;

typedef int loski_SocketRecvFlag;
#define LOSKI_SOCKRCV_PEEKONLY	MSG_PEEK
#define LOSKI_SOCKRCV_WAITALL	MSG_WAITALL
#define LOSKI_SOCKRCV_CUSTOM

typedef int loski_SocketWay;
#define LOSKI_SOCKWAY_IN	SHUT_RD;
#define LOSKI_SOCKWAY_OUT	SHUT_WR;
#define LOSKI_SOCKWAY_BOTH	SHUT_RDWR;
#define LOSKI_SOCKWAY_CUSTOM


/* Names */

#include <netdb.h>

typedef struct loski_AddressFound {
	struct addrinfo *results;
	struct addrinfo *next;
	int nexttype;
} loski_AddressFound;

typedef int loski_AddressNameFlag;
#define LOSKI_ADDRNAME_LOCAL	NI_NOFQDN
#define LOSKI_ADDRNAME_DGRM	NI_DGRAM
#define LOKSI_ADDRNAME_CUSTOM


#include "netlibapi.h"


#endif
