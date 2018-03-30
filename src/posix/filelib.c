#include "luaosi/filelib.h"


#include "luaosi/lfilelib.h"
#include "luaosi/fdlib.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

static losi_ErrorCode createpipe (losi_FileHandle *file,
                                  const char *path,
                                  losi_FileAccessPerm perm)
{
	file->fd = mkfifo(path, perm);
	if (file->fd != -1) return LOSI_ERRNONE;
	switch (errno) {
		case EACCES: return LOSI_ERRDENIED;
		case EEXIST: return LOSI_ERRINUSE;
		case ELOOP:
		case ENAMETOOLONG: return LOSI_ERRTOOMUCH;
		case ENOENT: return LOSI_ERRNOTFOUND;
		case ENOSPC: return LOSI_ERRNORESOURCES;
		case ENOTDIR: return LOSI_ERRUNAVAILABLE;
		case EROFS: return LOSI_ERRREFUSED;
	}
	return LOSI_ERRUNSPECIFIED;
}

static losi_ErrorCode getopenerr ()
{
	switch (errno) {
		case EACCES: return LOSI_ERRDENIED;
		case EEXIST:
		case EISDIR: return LOSI_ERRINUSE;
		case ELOOP:
		case ENAMETOOLONG:
		case EOVERFLOW: return LOSI_ERRTOOMUCH;
		case EMFILE:
		case ENFILE:
		case ENOSPC: return LOSI_ERRNORESOURCES;
		case ENOENT: return LOSI_ERRNOTFOUND;
		case ENOTDIR: return LOSI_ERRUNAVAILABLE;
		case ENXIO: return LOSI_ERRUNREACHABLE;
		case EROFS:
		case ETXTBSY: return LOSI_ERRREFUSED;
		case EAGAIN: return LOSI_ERRUNFULFILLED;
		case ENOMEM: return LOSI_ERRNORESOURCES;
		case EINVAL: return LOSI_ERRINVALID;
		case EIO: return LOSI_ERRSYSTEMFAIL;
		case ENOSR: return LOSI_ERRSYSTEMDOWN;
	}
	return LOSI_ERRUNSPECIFIED;
}

#define LOSI_FILEOPEN_READWRITE (LOSI_FILEOPEN_READ|LOSI_FILEOPEN_WRITE)

LOSIDRV_API losi_ErrorCode losiF_openfile (losi_FSysDriver *drv,
                                           losi_FileHandle *file,
                                           const char *path,
                                           losi_FileOpenFlags flags,
                                           losi_FileAccessPerm perm)
{
	int oflag;
	switch (flags&LOSI_FILEOPEN_READWRITE) {
		case LOSI_FILEOPEN_READ: oflag = O_RDONLY; break;
		case LOSI_FILEOPEN_WRITE: oflag = O_WRONLY; break;
		case LOSI_FILEOPEN_READWRITE: oflag = O_RDWR;
			if (!(flags&LOSI_FILEOPEN_PIPE)) break;
		default: return LOSI_ERRINVALID;
	}
	if (!(flags&LOSI_FILEOPEN_PIPE)) {
		if (oflag != O_RDONLY) {
			if (flags&LOSI_FILEOPEN_APPEND) oflag |= O_APPEND;
			if (flags&LOSI_FILEOPEN_TRUNC) oflag |= O_TRUNC;
		}
		if (flags&LOSI_FILEOPEN_CREATE) oflag |= O_CREAT;
		if (flags&LOSI_FILEOPEN_NEW) oflag |= (O_CREAT|O_EXCL);
	} else if (flags&LOSI_FILEOPEN_NEW) {
		losi_ErrorCode err = createpipe(file, path, perm);
		if (err) return err;
	}
	do {
		file->fd = open(path, oflag, perm);
		if (file->fd != -1) return LOSI_ERRNONE;
		if ((errno == ENOENT) && (flags&(LOSI_FILEOPEN_CREATE|LOSI_FILEOPEN_PIPE))) {
			losi_ErrorCode err = createpipe(file, path, perm);
			if (err) return err;
			errno = EINTR;
		}
	} while (errno == EINTR);
	return getopenerr();
}

LOSIDRV_API losi_ErrorCode losiF_pipefile (losi_FSysDriver *drv,
                                           losi_FileHandle *rfile,
                                           losi_FileHandle *wfile)
{
	int fds[2];
	if (pipe(fds) == 0) {
		rfile->fd = fds[0];
		wfile->fd = fds[1];
		return LOSI_ERRNONE;
	}
	switch (errno) {
		case EMFILE:
		case ENFILE: return LOSI_ERRNORESOURCES;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_IntUniqueId losiF_getfileid (losi_FSysDriver *drv,
                                              losi_FileHandle *file)
{
	return file->fd;
}

LOSIDRV_API losi_ErrorCode losiF_setfileopt (losi_FSysDriver *drv,
                                             losi_FileHandle *file,
                                             losi_FileOption option,
                                             int value)
{
	switch (option) {
		case LOSI_FILEOPT_BLOCKING: return losiFD_setnonblock(file->fd, value);
	}
	return LOSI_ERRUNSUPPORTED;
}

LOSIDRV_API losi_ErrorCode losiF_getfileopt (losi_FSysDriver *drv,
                                             losi_FileHandle *file,
                                             losi_FileOption option,
                                             int *value)
{
	switch (option) {
		case LOSI_FILEOPT_BLOCKING: return losiFD_getnonblock(file->fd, value);
	}
	return LOSI_ERRUNSUPPORTED;
}

LOSIDRV_API losi_ErrorCode losiF_writefile (losi_FSysDriver *drv,
                                            losi_FileHandle *file,
                                            const char *data,
                                            size_t size,
                                            size_t *bytes)
{
	do {
		ssize_t err = write(file->fd, data, size);
		if (err >= 0) {
			*bytes = (size_t)err;
			return LOSI_ERRNONE;
		}
	} while (errno == EINTR);
	switch (errno) {
#if EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		case EWOULDBLOCK: return LOSI_ERRUNFULFILLED;
		case EBADF: return LOSI_ERRINVALID;
		case EFBIG: return LOSI_ERRTOOMUCH;
		case EIO: return LOSI_ERRSYSTEMFAIL;
		case ENOSPC: return LOSI_ERRNORESOURCES;
		case EPIPE: return LOSI_ERRABORTED;
		case ENOBUFS: return LOSI_ERRNOMEMORY;
		case ENXIO: return LOSI_ERRREFUSED;
		case ECONNRESET:
		case EACCES:
		case ENETDOWN:
		case ENETUNREACH: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiF_readfile (losi_FSysDriver *drv,
                                           losi_FileHandle *file,
                                           char *buffer,
                                           size_t size,
                                           size_t *bytes)
{
	do {
		ssize_t err;
		if (size == 0) return LOSI_ERRINVALID;
		err = read(file->fd, buffer, size);
		if (err >= 0) {
			*bytes = (size_t)err;
			return LOSI_ERRNONE;
		}
	} while (errno == EINTR);
	switch (errno) {
#if EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		case EWOULDBLOCK: return LOSI_ERRUNFULFILLED;
		case EBADF: return LOSI_ERRINVALID;
		case EIO: return LOSI_ERRSYSTEMFAIL;
		case EOVERFLOW: return LOSI_ERRTOOMUCH;
		case ENOMEM:
		case ENOBUFS: return LOSI_ERRNOMEMORY;
		case ENXIO: return LOSI_ERRREFUSED;
		case ECONNRESET:
		case ENOTCONN:
		case ETIMEDOUT: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API losi_ErrorCode losiF_closefile (losi_FSysDriver *drv,
                                            losi_FileHandle *file)
{
	do if (close(file->fd) == 0) return LOSI_ERRNONE;
	while (errno == EINTR);
	switch (errno) {
		case EIO: return LOSI_ERRSYSTEMFAIL;
		case EBADF: return LOSI_ERRUNEXPECTED;
	}
	return LOSI_ERRUNSPECIFIED;
}

LOSIDRV_API int losiF_getfileprocstrm (void *udata, losi_ProcStream *fd)
{
	LuaFile *lf = (LuaFile *)udata;
	if (lf->refs == 0) return 0;
	*fd = lf->handle.fd;
	return 1;
}

LOSIDRV_API losi_ErrorCode losiF_getfileevtsrc (void *udata, int newref,
                                                losi_EventSource *src,
                                                losi_EventFlags evtflags)
{
	LuaFile *lf = (LuaFile *)udata;
	if (lf->refs == 0) return LOSI_ERRCLOSED;
	if (newref) ++(lf->refs);
	*src = lf->handle.fd;
	return LOSI_ERRNONE;
}

LOSIDRV_API void losiF_freefileevtsrc (void *udata)
{
	LuaFile *lf = (LuaFile *)udata;
	--(lf->refs);
	assert(lf->refs > 0);
}
