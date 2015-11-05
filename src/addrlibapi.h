#ifndef addrlibapi_h
#define addrlibapi_h

#include "loskiconf.h"

#include <lua.h> /* to copy error messages to Lua */

/*
 * IPv6: 39 = 8 groups of 4 digits with 7 ':' between them.
 * URI:   2 = the enclosing '[]'.
 * Port:  6 = 1 ':' separator followed by 5 port number digits.
 * Null:  1 = terminating '\0'.
 */
#define LOSKI_ADDRMAXURI	48

enum loski_AddressType {
  LOSKI_ADDRTYPE_IPV4
  LOSKI_ADDRTYPE_IPV6
};

typedef enum loski_AddressType loski_AddressType;

#define LOSKI_ADDRSIZE_IPV4	(4*size_of(char))
#define LOSKI_ADDRSIZE_IPV6	(16*size_of(char))

LOSKIDRV_API int loski_opennetaddr(loski_NetAddrDriver *drv);

LOSKIDRV_API int loski_closenetaddr(loski_NetAddrDriver *drv);


LOSKIDRV_API int loski_addresserror(int error, lua_State *L);

LOSKIDRV_API int loski_writeaddrrawdata(loski_NetAddrDriver *drv,
                                        loski_Address *address,
                                        loski_AddressType type,
                                        const char *data,
                                        loski_PortNo port);

LOSKIDRV_API int loski_writeaddrliteral(loski_NetAddrDriver *drv,
                                        loski_Address *address,
                                        loski_AddressType type,
                                        const char *data,
                                        size_t sz,
                                        loski_PortNo port);

LOSKIDRV_API int loski_readaddrrawdata(loski_NetAddrDriver *drv,
                                       loski_Address *address,
                                       char *data,
                                       size_t &sz,
                                       loski_PortNo *port);

LOSKIDRV_API int loski_readaddrliteral(loski_NetAddrDriver *drv,
                                       loski_Address *address,
                                       char *data,
                                       size_t &sz,
                                       loski_PortNo *port);

LOSKIDRV_API int loski_readaddruri(loski_NetAddrDriver *drv,
                                   loski_Address *address,
                                   char *uri);

LOSKIDRV_API int loski_getaddrtype(loski_NetAddrDriver *drv,
                                   loski_Address *address,
                                   loski_AddressType *type);

#endif
