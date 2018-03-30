#ifndef luaosi_filedef_h
#define luaosi_filedef_h


#include "luaosi/proclib.h"
#include "luaosi/evtlib.h"

#include <sys/stat.h>

#define LOSI_DISABLE_FSDRV

typedef struct losi_FileHandle {
	int fd;
} losi_FileHandle;

typedef int losi_FileAccessPerm;
#define LOSI_FILEPERM_USRREAD 	S_IRUSR
#define LOSI_FILEPERM_USRWRITE	S_IWUSR
#define LOSI_FILEPERM_USREXEC 	S_IXUSR
#define LOSI_FILEPERM_GRPREAD 	S_IRGRP
#define LOSI_FILEPERM_GRPWRITE	S_IWGRP
#define LOSI_FILEPERM_GRPEXEC 	S_IXGRP
#define LOSI_FILEPERM_OTHREAD 	S_IROTH
#define LOSI_FILEPERM_OTHWRITE	S_IWOTH
#define LOSI_FILEPERM_OTHEXEC 	S_IXOTH
#define LOSI_FILEPERM_CUSTOM

#define LOSI_ENABLE_FILEPROCSTREAM

LOSIDRV_API int losiF_getfileprocstrm (void *udata, losi_ProcStream *fd);

#define LOSI_ENABLE_FILEEVENTS

LOSIDRV_API losi_ErrorCode losiF_getfileevtsrc (void *udata, int newref,
                                                losi_EventSource *src,
                                                losi_EventFlags evtflags);
LOSIDRV_API void losiF_freefileevtsrc (void *udata);


#endif
