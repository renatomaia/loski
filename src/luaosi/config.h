#ifndef luaosi_config_h
#define luaosi_config_h

#include <luaconf.h>

#ifndef LOSILIB_API
#define LOSILIB_API LUALIB_API
#endif

#ifndef LOSIDRV_API
#define LOSIDRV_API
#endif

#ifndef LOSI_VALUEID
#define LOSI_VALUEID LUA_INTEGER
#endif

#define LOSI_PREFIX "losi:"

typedef LOSI_VALUEID losi_IntUniqueId;
typedef void * (*losi_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


#endif
