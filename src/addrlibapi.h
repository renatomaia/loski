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

LOSKIDRV_API int loski_writeaddrbytes(loski_Address *address,
                                      loski_AddressType type,
                                      const char *data,
                                      loski_AddressPort port);

LOSKIDRV_API int loski_writeaddrliteral(loski_Address *address,
                                        loski_AddressType type,
                                        const char *data,
                                        size_t sz,
                                        loski_AddressPort port);

LOSKIDRV_API loski_AddressType loski_addrtype(loski_Address *address);

LOSKIDRV_API loski_AddressPort loski_addrport(loski_Address *address);

LOSKIDRV_API const char *loski_addrbytes(loski_Address *address, size_t *sz);

LOSKIDRV_API const char *loski_addrliteral(loski_Address *address, char *data);

#endif
