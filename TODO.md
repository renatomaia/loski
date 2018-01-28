TODO
====

http://pauillac.inria.fr/~xleroy/linuxthreads/faq.html#J

Not Priorized:
- Add support to wait for child process termination in 'event' module.
- Avoid realloc child process structure on removal, which is called from signal handler. Only realloc on insertion, just like Lua tables.
- Object to represent buffers (LuaMemory)
- Support for (named) pipes
- Support for async file operations
- Support for file system operations (Lua File System)
- Support for file system events
- Support to obtain the system properties like a name (windows, linux, etc.) or version.
- Document C API and C libraries dependencies.
