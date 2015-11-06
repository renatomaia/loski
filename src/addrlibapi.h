#ifndef addrlibapi_h
#define addrlibapi_h

#include "loskiconf.h"

#include <lua.h> /* to copy error messages to Lua */

enum loski_AddressType {
  LOSKI_ADDRTYPE_IPV4,
  LOSKI_ADDRTYPE_IPV6
};
typedef enum loski_AddressType loski_AddressType;

#define LOSKI_ADDRSIZE_IPV4	(4*sizeof(char))
#define LOSKI_ADDRSIZE_IPV6	(16*sizeof(char))

#define LOSKI_ADDRMAXPORT	65535
#define LOSKI_ADDRMAXLITERAL	40  /* 8 groups of 4 digits + 7 ':' + '\0' */


LOSKIDRV_API int loski_initaddress(loski_Address *address,
                                   loski_AddressType type);


LOSKIDRV_API void loski_setaddrport(loski_Address *address,
                                    loski_AddressPort port);

LOSKIDRV_API void loski_setaddrbytes(loski_Address *address, const char *data);

LOSKIDRV_API int loski_setaddrliteral(loski_Address *address,
                                      const char *data,
                                      size_t sz);


LOSKIDRV_API loski_AddressType loski_getaddrtype(loski_Address *address);

LOSKIDRV_API loski_AddressPort loski_getaddrport(loski_Address *address);

LOSKIDRV_API const char *loski_getaddrbytes(loski_Address *address, size_t *sz);

LOSKIDRV_API const char *loski_getaddrliteral(loski_Address *address,
                                              char *data);

#endif
