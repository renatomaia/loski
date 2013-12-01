loski
=====

Operating System Kernel Interface for Lua

The main purpose of this project is to provide Lua libraries to access features provided by the platform's underlying operating system (OS) kernel, such as process control, network access, file system, etc.

The initial focus is on the design of a simple API that allows different implementations over various platforms (POSIX, Linux, Windows, etc.) yet good integration between the provided features.

The implemenation of the libraries are heavily based (i.e. shamelessly stolen from ;-) on other Lua libraries like LuaSocket, LuaFileSystem and others.
