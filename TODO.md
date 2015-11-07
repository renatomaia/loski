TODO
====

http://pauillac.inria.fr/~xleroy/linuxthreads/faq.html#J

Not Priorized:
- Treat drivers as external libs by using include with angle brackets.
- Review 'process' and 'event' lib to apply new techniques from 'network' lib.
- Review which driver errors should become errors and which shall be 'nil, errmsg'.
- Object to represent buffers (Lua Buffers)
- Add IPv6 support (include option in 'network.create').
- Support for (named) pipes
- Support for child process termination event
- Support for async file operations
- Support for file system operations (Lua File System)
- Support for file system events
- Support to obtain the system properties like a name (windows, linux, etc.) or version.