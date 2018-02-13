Contents
========

1. [Time Facilities](#time-facilities)
2. [Process Facilities](#process-facilities)
3. [Network Facilities](#network-facilities)
3. [Event Facilities](#event-facilities)
4. [Error Messages](#error-messages)

Index
=====

- [`event.watcher`](#watcher--errmsg--eventwatcher-)
- [`watcher:set`](#succ--errmsg--watcherset-source--events)
- [`watcher:wait`](#map--errmsg--watcherwait-timeout)
- [`watcher:close`](#succ--errmsg--watcherclose-)

- [`network.address`](#address--networkaddress-data--port--mode)
- [`network.getname`](#name--service--networkgetname-address--mode)
- [`network.resolve`](#next--errmsg--networkresolve-name--service--mode)
- [`network.socket`](#socket--errmsg--networksocket-type--domain)
- [`network.type`](#type--networktype-value)
- [`socket:accept`](#connection--errmsg--socketaccept-address)
- [`socket:bind`](#success--errmsg--socketbind-address)
- [`socket:close`](#success--errmsg--socketclose-)
- [`socket:connect`](#success--errmsg--socketconnect-address)
- [`socket:getaddress`](#address--errmsg--socketgetaddress-site--address)
- [`socket:getoption`](#value--errmsg--socketgetoption-name)
- [`socket:listen`](#success--errmsg--socketlisten-backlog)
- [`socket:receive`](#data--errmsg--socketreceive-size--mode--address)
- [`socket:send`](#sent--errmsg--socketsend-data--i--j--address)
- [`socket:setoption`](#success--errmsg--socketsetoption-name-value)
- [`socket:shutdown`](#success--errmsg--socketshutdown-mode)
- [`socket.type`](#type--sockettype)

- [`process.create`](#proc--processcreate-cmd--)
- [`process.exitval`](#number--errmsg--processexitval-proc)
- [`process.kill`](#succ--errmsg--processkill-proc)
- [`process.status`](#status--processstatus-proc)

- [`time.now`](#seconds--timenow-)
- [`time.sleep`](#timesleep-delay)

Manual
======

Time Facilities
---------------

### `seconds = time.now ()`

Returns the number of seconds since some given start time (the "epoch"), just like `os.time()`, but with sub-second precision.

### `time.sleep (delay)`

Suspends the execution for `delay` seconds.

Network Facilities
------------------

### `address = network.address ([data [, port [, mode]]])`

Creates an address structure.

If no parameters are provided the structure created is initilized with address `0.0.0.0:0` (IPv4).

`data` is a string with the information to be stored in the structure created.

If only `data` is provided, it must be a literal address as formatted inside a URI, like `"192.0.2.128:80"` (IPv4) or `"[::ffff:c000:0280]:80"` (IPv6).

If `port` is provided, `data` is a host address and `port` is a port number to be used to initialize the address structure.
The string `mode` controls whether `data` is text (literal) or binary.
It may be the string `"b"` (binary data) or `"t"` (text data).
The default is `"t"`.

Returns a structure that provides the following fields:

`type`
:	is either the string `"ipv4"` or `"ipv6"` to indicate the address is a IPv4 or IPv6 address, respectively.
	When the value of this field changes, the whole address data is set to null values: `0.0.0.0:0` for `"ipv4"`, or `[::]:0` for `"ipv6"`.

`literal`
:	is the text (literal) representation of the address, like `"192.0.2.128"` (IPv4) or `"::ffff:c000:0280"` (IPv6).

`binary`
:	is the binary representation of the address, like `"\192\0\2\128"` (IPv4) or `"\0\0\0\0\0\0\0\0\0\0\255\255\192\0\2\128"` (IPv6).

`port`
:	is the port number of the address.

Moreover, you can pass the object to the standard function `tostring` to obtain the address as a string inside a URI, like `"192.0.2.128:80"` (IPv4) or `[::ffff:c000:0280]:80` (IPv6).

### `next [, errmsg] = network.resolve (name [, service [, mode]])`

Searches for the addresses for a network node with name `name`.
If `name` is `nil`, the loopback address is searched.
If `name` is `"*"`, the wildcard address is searched.
If some address is found, returns an iterator function with the following usage pattern:

	[address, socktype, more =] next ([address])

Otherwise, it returns `nil` plus an error message.

Each time the iterator function is called, returns one address found for node with `name`, followed by the type of the socket to be used to connect to the address, and a boolean indicating if there is more results to be returned in future calls.
If an address structure is provided as `address`, it is used to store the result; otherwise a new address structure is created.

`service` indicates the port number or service name to be used to resolve the port number of the resulting addresses.
When `service` is absent, the port zero is used in the results.
The string `mode` defines the search domain. 
It can be contain any of the following characters:

- `4`: for IPv4 addresses.
- `6`: for IPv6 addresses.
- `m`: for IPv4-mapped addresses.
- `d`: for addresses for `datagram` sockets.
- `s`: for addresses for `stream` or `listen` sockets.

When neither `4` nor `6` are provided, the search only includes addresses of the same type configured in the local machine.
When neither `d` nor `s` are provided, the search behaves as if both `d` and `s` were provided.
By default, `mode` is the empty string.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"not found"`
- `"no system memory"`
- `"system error"`

As an example, the following loop will iterate over all the addresses found for service named 'ssh' on node named `www.lua.org`, using the same address object:

	next = assert(network.resolve("www.lua.org", "ssh"))
	for addr, scktype in next, address.create() do
		print(addr, scktype)
	end

The next example gets only the first address found.

	addr, scktype = network.resolve("www.lua.org", "http", "s")()

Yet another example that collects all address found in new address objects.

	list = {}
	for addr, scktype in network.resolve("www.lua.org", "http", "s") do
		list[#list+1] = addr
	end

Finally, an example that fills existing addreses objects with the results

	next = network.resolve("www.lua.org", "http", "s")
	repeat until not select(3, next(getSomeAddressObject()))

### `name [, service] = network.getname (address [, mode])`

Searches for a network name for `address`.
If `address` is an address structure, it returns a host name and a port service name for the address.
If `address` is a number, it returns the service name for that port number.
If `address` is a string, it returns a canonical name for that network name.

In case of errors, it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"not found"`
- `"no system memory"`
- `"system error"`

### `socket [, errmsg] = network.socket (type [, domain])`

Creates a socket, of the type specified in the string `type`.

The `type` string can be any of the following:

`"datagram"`
:	datagram socket (UDP).

`"stream"`
:	stream socket (TCP).

`"listen"`
:	socket to accept stream socket connections (TCP).

The `domain` string defines the socket's address domain (or family) and can be any of the following:

`"ipv4"`
:	IPv4 addresses. (the default)

`"ipv6"`
:	IPv6 addresses.

It returns a new socket handle, or, in case of errors, `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"no resources"` (no descriptors available)
- `"access denied"`
- `"no system memory"`

### `type = network.type (value)`

Returns the string `"address"` if `value` is an address structure, or `"socket"` if `value` is a socket handle; otherwise it returns `nil`.

### `type = socket.type`

The type of `socket`, which can be: `"datagram"` `"stream"`, `"listen"`.
This field is read-only.

### `success [, errmsg] = socket:close ()`

Closes `socket`.
Note that sockets are automatically closed when their handles are garbage collected, but that takes an unpredictable amount of time to happen. 

In case of success, this function returns `true`.
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"unfulfilled"` (interrupted by signal)
- `"system error"`

### `success [, errmsg] = socket:bind (address)`

Binds `socket` to the local address provided as `address`.

In case of success, this function returns `true`.
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"in use"` (socket is connected)
- `"unavailable"` (address is in use)
- `"unreachable"` (address is not local)
- `"no system memory"`

### `address [, errmsg] = socket:getaddress ([site [, address]])`

Returns the address associated with `socket`, as indicated by `site`, which can be:

`"this"`
:	The socket's address (the default).

`"peer"`
:	The socket's peer address.

If `address` is provided, it is the address structure used to store the result.

In case of errors, it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"closed"` (socket is not connected)
- `"no system memory"`

### `value [, errmsg] = socket:getoption (name)`

Returns the value of option `name` of `socket`, or, in case of errors, `nil` plus an error message.
There available options are:

- `"blocking"`: is `false` for a non-blocking socket, or `true` otherwise.
- `"reuseaddr"`: is `true` when address reuse is allowed for `socket`, or `false`otherwise.
- `"dontroute"`: is `true` when routing facilities are disabled for `socket`, or `false` otherwise.

Sockets of type `"stream"` accept the additional options:

- `"linger"`: is the number of seconds that `socket:close` can be delayed when there is pending data to be sent.
- `"keepalive"`: is `true` when periodic transmission of messages is enabled for `socket`, or `false` otherwise.
- `"nodelay"`: is `true` when coalescing of small segments shall be avoided in `socket`, or `false` otherwise.

Sockets type `"datagram"` accept the additional option:

- `"broadcast"`: is `true` when sending of broadcast messages are permited for `socket`, or `false` otherwise.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"access denied"`
- `"no system memory"`

### `success [, errmsg] = socket:setoption (name, value)`

Sets the option `name` for `socket`.
The available options are the same as defined in operation [`socket:getoption`](#value--errmsg--socketgetoption-name).

In case of success, this function returns `true`.
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"in use"` (socket is connected)
- `"too much"` (value too large for option)

### `success [, errmsg] = socket:listen ([backlog])`

Starts listening for new connections on socket.
`backlog` is a hint for the underlying system about the suggested number of outstanding connections that shall be kept in the socket's listen queue.
By default `backlog` is 32.

This operation is only available for `listen` sockets.

In case of success, this function returns `true`.
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"access denied"`
- `"no resources"`

### `connection [, errmsg] = socket:accept ([address])`

Accepts a new pending connection on `socket`.
An address structure can be provided as `address` to be filled with the address of the accepted connection's peer socket.

This operation is only available for `listen` sockets.

In case of success, this function returns a new `stream` socket for the accepted connection.
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"unfulfilled"`
- `"aborted"` (a connection has been aborted)
- `"no resources"`
- `"no system memory"`

### `success [, errmsg] = socket:connect (address)`

Binds `socket` to the peer address provided as `address`.
This operation is not available for `listen` sockets.

In case of success, this function returns `true`.
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"unfulfilled"`
- `"in use"` (socket is connected or address in use)
- `"unavailable"` (address not available)
- `"unreachable"` (address in unreachable)
- `"refused"` (connection refused by peer)
- `"aborted"` (connection reset by peer)
- `"timeout"`
- `"system down"`
- `"no system memory"`

### `sent [, errmsg] = socket:send (data [, i [, j [, address]]])`

Sends the substring of `data` that starts at `i` and continues until `j`; `i` and `j` can be negative.
If `j` is absent, then it is assumed to be equal to -1 (which is the same as the string length).

This operation is not available for `listen` sockets.
For disconnected datagram sockets, it is necessary to provide `address` with the peer address the data is destined.

In case of success, this function returns the number of bytes from `data` effectivelly sent through `socket`.
Otherwise it returns `nil` plus an error message.

__Note__: if `data` is a [memory](https://github.com/renatomaia/lua-memory), it is not converted to a Lua string prior to have its specified contents transfered.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"unfulfilled"`
- `"too much"` (message too large)
- `"closed"` (connection is closed)
- `"aborted"` (connection reset by peer)
- `"access denied"`
- `"unreachable"` (peer is unreachable)
- `"no system memory"`
- `"system down"`
- `"system error"`

### `data [, errmsg] = socket:receive (size [, mode [, address]])`

Receives up to `size` bytes from socket `socket`, using the mode specified in the string `mode`.
The mode string can be contain any of the following characters:

- `p`: only peeks at an incoming message, leaving the returned data as unread in `socket`.
- `a`: only returns successfully when all the `size` bytes were received (only for `stream` sockets).

By default, `mode` is the empty string, which disables all options.

This operation is not available for `listen` sockets.
For disconnected datagram sockets, it is possible to provide `address` with an address structure to be filled with the address of the peer socket from which the received data were originated.

In case of success, this function returns the `data` effectivelly received from `socket`.
Otherwise it returns `nil` plus an error message.

__Note__: if `size` is not a number of bytes but a [memory](https://github.com/renatomaia/lua-memory), the size of the memory is used as `size`.
Moreover, in case of success, the number of bytes actually received are returned instead of a string with the data.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"unfulfilled"`
- `"timeout"`
- `"closed"` (connection is closed)
- `"aborted"` (connection reset by peer)
- `"no resources"`
- `"no system memory"`
- `"system down"` (network interface is down)
- `"system error"`

### `success [, errmsg] = socket:shutdown ([mode])`

Shuts down `socket` connection in one or both directions, as specified in the string `mode`.
The mode string can be any of the following:

- `send`: disables further send operations.
- `receive`: disables further receive operations.
- `both`: disables both send and receive operations. (the default)

This operation is only available for `stream` sockets.

In case of success, this function returns `true`.
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"closed"` (socket is not connected)
- `"no resources"`

Process Facilities
---------------

### `proc = process.create (cmd [, ...])`

This function creates a new process.
`cmd` is the path of the executable image for the new process.
Every other extra arguments are strings to be used as command-line arguments for the executable image of the new process.
Alternatively, `cmd` can be a table with the fields described below.
Unless stated otherwise, when one of these field are not defined in table `cmd`, or `cmd` is a string, the new process inherits the characteristics of the current process, like the current directory, the environment variables, or standard files.

`execfile`
:	path of the executable image for the new process.
	This field is required.

`runpath`
:	path of the current directory of the new process.

`stdin`
:	file to be set as the standard input of the new process.

`stdout`
:	file to be set as the standard output of the new process.

`stderr`
:	file to be set as the standard error output of the new process.

`arguments`
:	table with the sequence of command-line arguments for the executable image of the new process.
	When this field is not provided, the new process's executable image receives no arguments.

`environment`
:	table mapping environment variable names to the values they must assume for the new process.
	If this field is provided, only the variables defined will be available for the new process.

It returns a handle for the new process, or, in case of errors, `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"access denied"`
- `"too much"`
- `"no resources"`
- `"no system memory"`

### `status = process.status (proc)`

Returns the status of process `proc`, as a string:
"running", if the process is running;
"suspended", if the process is suspended;
and "dead" if the process terminated.
In the last case, operation [`process.exitval`]{#process.exitval} can be used to get the exit value of the process.

### `number [, errmsg] = process.exitval (proc)`

Returns the exit value of process `proc` as a number in the range [0; 255], or, in case of errors, `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"unfulfilled"` (process has not terminated yet)
- `"aborted"` (process terminated before completion)

### `succ [, errmsg] = process.kill (proc)`

Forces the termination of process `proc`.
In case of success, this function returns `true`.
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"access denied"`

Event Facilities
----------------

### `watcher [, errmsg] = event.watcher ()`

Creates a new event watcher.

In case of success, this function returns the event watcher.
Otherwise it returns `nil` plus an error message.

### `succ [, errmsg] = watcher:set (source [, events])`

Defines which events should be watched from `source` object.

`source` is either a file, socket or process, depending on the LOSI support for the current platform.

`events` is a string containing the following characters defining which events should be watched from `source`:

- `r`: availability to read data from the `source`
- `w`: availability to write data to the `source`.

If `events` is not provided or is the empty string, the `source` will not be watched by the `watcher`.

When `source` is a process, availability to read data indicates the process terminated and it is possible to obtain its exit value ([`process.exitval`](#number--errmsg--processexitval-proc)).

In case of success, this function returns `true`.
Otherwise it returns `nil` plus an error message.

### `map [, errmsg] = watcher:wait ([timeout])`

Waits until for any of the events watched by `watcher` happens.

`timeout` is an optional timeout for the operation.
If no `timeout` is provided, the call waits indefinetaly for one of the wached events to happen.

It returns a table mapping sources to a string indicating the events that happened, like the parameter `events` of ([`watcher:set`](#succ--errmsg--watcherset-source--events)).
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"unfulfilled"` (interrupted by signal)

### `succ [, errmsg] = watcher:close ()`

Closes `watcher` and releases all its resources, including references to watched sources, which might be collected if no other references to them exist.
Note that watchers are automatically closed when their handles are garbage collected, but that takes an unpredictable amount of time to happen. 

In case of success, this function returns `true`.
Otherwise it returns `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#error-mesages).

- `"unfulfilled"` (interrupted by signal)
- `"system error"`

Error Messages
--------------

The following messages can be returned by the I/O operations (after a `nil`) to indicate expected errors:

`"unfulfilled"`
:	operation was not able to complete yet (try again).

`"closed"`
:	resource is closed.

`"in use"`
:	resource is already in use.

`"not found"`
:	resource was not found.

`"unavailable"`
:	resource is unavailable.

`"unreachable"`
:	resource is unreachable.

`"aborted"`
:	resource reclaimed abruptly.

`"refused"`
:	resource refused.

`"no resources"`
:	resources were exhausted.

`"too much"`
:	resource limits were extrapolated.

`"timeout"`
:	insufficient time to complete.

`"access denied"`
:	insufficient permission.

`"no system memory"`
:	insufficient memory in the underlying system.

`"system down"`
:	underlying system is down.

`"system error"`
:	underlying system error.

Moreover, any operation shall raise the following errors:

`"invalid operation"`
:	operation is not valid for this object or parameters.

`"unsupported"`
:	operation is not supported in the current system.

`"unexpected error"`
:	underlying system raised an unexpected error, usually due to poor LOSI support for the particular platform.

`"unspecified error"`
:	underlying system raised a non-conforming error, usually due to platform's poor adherence to standards.

`"unknown error (<number>)"`
:	underlying system raised an illegal error value (indicated in the message) due to wrong LOSI implementation or build.
