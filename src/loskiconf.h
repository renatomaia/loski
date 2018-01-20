#ifndef loskiconf_h
#define loskiconf_h

#include <luaconf.h>

#ifndef LOSKILIB_API
#define LOSKILIB_API LUALIB_API
#endif

#ifndef LOSKIDRV_API
#define LOSKIDRV_API
#endif

#ifndef LOSKI_VALUEID
#define LOSKI_VALUEID LUA_INTEGER
#endif

#define LOSKI_PREFIX "loski:"

typedef LOSKI_VALUEID loski_IntUniqueId;
typedef void * (*loski_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


#endif
