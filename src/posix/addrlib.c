#include "addrlib.h"

#include <string.h>
#include <arpa/inet.h>

/* TODO: ASSERT(LOSKI_ADDRSIZE_IPV4 == sizeof(in_addr_t)) */

#define MAXLITERAL	16

LOSKIDRV_API int loski_initaddress(loski_Address *address,
                                   loski_AddressType type)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	if (type != LOSKI_ADDRTYPE_IPV4) return 0;
	memset(address, 0, sizeof(loski_Address));
	addr->sin_family = AF_INET;
	return 1;
}

LOSKIDRV_API void loski_setaddrport(loski_Address *address,
                                    loski_AddressPort port)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	addr->sin_port = htons(port);
}

LOSKIDRV_API void loski_setaddrbytes(loski_Address *address,
                                     const char *data)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	memcpy(&(addr->sin_addr.s_addr), data, LOSKI_ADDRSIZE_IPV4);
}

LOSKIDRV_API int loski_setaddrliteral(loski_Address *address,
                                      const char *data,
                                      size_t sz)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	char b[MAXLITERAL];
	if (sz >= MAXLITERAL) return 0;  /* too long! */
	memcpy(b, data, sz);
	b[sz] = '\0';
	return inet_aton(b, &addr->sin_addr);
}

LOSKIDRV_API loski_AddressType loski_getaddrtype(loski_Address *address)
{
	return LOSKI_ADDRTYPE_IPV4;
}

LOSKIDRV_API loski_AddressPort loski_getaddrport(loski_Address *address)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	return ntohs(addr->sin_port);
}

LOSKIDRV_API const char *loski_getaddrbytes(loski_Address *address, size_t *sz)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	if (sz) *sz = LOSKI_ADDRSIZE_IPV4;
	return (const char *)(&(addr->sin_addr.s_addr));
}

LOSKIDRV_API const char *loski_getaddrliteral(loski_Address *address,
                                              char *data)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)address;
	return inet_ntoa(addr->sin_addr);
}
