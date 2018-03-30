#ifndef luaosi_filelib_h
#define luaosi_filelib_h


#include "luaosi/filedef.h"

#include "luaosi/config.h"
#include "luaosi/errors.h"

/* Library */

typedef void losi_FSysDriver;

#ifndef LOSI_DISABLE_FSDRV
LOSIDRV_API losi_ErrorCode losiF_initdrv (losi_FSysDriver *drv);

LOSIDRV_API void losiF_freedrv (losi_FSysDriver *drv);
#endif

/* Files */

#ifndef LOSI_FILEOPEN_CUSTOM
typedef enum losi_FileOpenFlags {
	LOSI_FILEOPEN_READ = 0x01,
	LOSI_FILEOPEN_WRITE = 0x02,
	LOSI_FILEOPEN_APPEND = 0x04,
	LOSI_FILEOPEN_TRUNC = 0x08,
	LOSI_FILEOPEN_CREATE = 0x10,
	LOSI_FILEOPEN_NEW = 0x20,
  LOSI_FILEOPEN_PIPE = 0x40
} losi_FileOpenFlags;
#endif

#ifndef LOSI_FILEPERM_CUSTOM
typedef enum losi_FileAccessPerm {
	LOSI_FILEPERM_USRREAD = 0x001,
	LOSI_FILEPERM_USRWRITE = 0x002,
	LOSI_FILEPERM_USREXEC = 0x004,
	LOSI_FILEPERM_GRPREAD = 0x008,
	LOSI_FILEPERM_GRPWRITE = 0x010,
	LOSI_FILEPERM_GRPEXEC = 0x020,
	LOSI_FILEPERM_OTHREAD = 0x040,
	LOSI_FILEPERM_OTHWRITE = 0x080,
	LOSI_FILEPERM_OTHEXEC = 0x100
} losi_FileAccessPerm;
#endif

#ifndef LOSI_FILEOPT_CUSTOM
typedef enum losi_FileOption {
  /* BASEFILE */
  LOSI_FILEOPT_BLOCKING,
} losi_FileOption;
#endif



LOSIDRV_API losi_ErrorCode losiF_openfile (losi_FSysDriver *drv,
                                           losi_FileHandle *file,
                                           const char *path,
                                           losi_FileOpenFlags flags,
                                           losi_FileAccessPerm perm);

LOSIDRV_API losi_ErrorCode losiF_pipefile (losi_FSysDriver *drv,
                                           losi_FileHandle *rfile,
                                           losi_FileHandle *wfile);

LOSIDRV_API losi_IntUniqueId losiF_getfileid (losi_FSysDriver *drv,
                                              losi_FileHandle *file);

LOSIDRV_API losi_ErrorCode losiF_setfileopt (losi_FSysDriver *drv,
                                             losi_FileHandle *file,
                                             losi_FileOption option,
                                             int value);

LOSIDRV_API losi_ErrorCode losiF_getfileopt (losi_FSysDriver *drv,
                                             losi_FileHandle *file,
                                             losi_FileOption option,
                                             int *value);

LOSIDRV_API losi_ErrorCode losiF_writefile (losi_FSysDriver *drv,
                                            losi_FileHandle *file,
                                            const char *data,
                                            size_t size,
                                            size_t *bytes);

LOSIDRV_API losi_ErrorCode losiF_readfile (losi_FSysDriver *drv,
                                           losi_FileHandle *file,
                                           char *buffer,
                                           size_t size,
                                           size_t *bytes);

LOSIDRV_API losi_ErrorCode losiF_closefile (losi_FSysDriver *drv,
                                            losi_FileHandle *file);


#endif
