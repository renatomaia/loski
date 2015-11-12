Contents
========

1. [Time Facilities](#time)
2. [Network Facilities](#network)
3. [Error Messages](#errmsg)


Index
=====

- [`network`](#network)
- [`network.address`](#network.address)
- [`network.getname`](#network.getname)
- [`network.resolve`](#network.resolve)
- [`network.socket`](#network.socket)
- [`network.type`](#network.type)
- [`socket:accept`](#socket:accept)
- [`socket:bind`](#socket:bind)
- [`socket:close`](#socket:close)
- [`socket:connect`](#socket:connect)
- [`socket:getaddress`](#socket:getaddress)
- [`socket:getoption`](#socket:getoption)
- [`socket:listen`](#socket:listen)
- [`socket:receive`](#socket:receive)
- [`socket:send`](#socket:send)
- [`socket:setoption`](#socket:setoption)
- [`socket:shutdown`](#socket:shutdown)
- [`socket.type`](#socket.type)

- [`time`](#time)
- [`time.now`](#time.now)
- [`time.sleep`](#time.sleep)


Manual
======

Time Facilities {#time}
---------------

### `seconds = time.now ()` {#time.now}

Returns the number of seconds since some given start time (the "epoch"), just like `os.time()`, but with sub-second precision.

### `time.sleep (delay)` {#time.sleep}

Suspends the execution for `delay` seconds.


Network Facilities {#network}
------------------

### `address = network.address ([data [, port [, mode]]])` {#network.address}

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

### `next [, errmsg] = network.resolve (name [, service [, mode]])` {#network.resolve}

Searches for the addresses for a network node with name `name`.
If `name` is `nil`, the loopback address is searched.
If `name` is `"*"`, the wildcard address is searched.
If some address is found, returns an iterator function with the following usage pattern:

	[address, socktype, more =] next ([address])

Otherwise, it returns `nil` plus a string describing the error.

Each time the iterator function is called, returns one address found for node with `name`, followed by the type of the socket to be used to connect to the address, and a boolean indicating if there is more results to be returned in future calls.
If an address structure is provided as `address`, it is used to store the result; otherwise a new address structure is created.

`service` indicates the port number or service name to be used to resolve the port number of the resulting addresses.
When `servname` is absent, the port zero is used in the results.
The string `mode` defines the search domain. 
It can be contain any of the following characters:

- `4`: for IPv4 addresses.
- `6`: for IPv6 addresses.
- `m`: for IPv4-mapped addresses.
- `d`: for addresses for `datagram` sockets.
- `s`: for addresses for `connection` or `listen` sockets (stream).

When neither `4` nor `6` are provided, the search only includes addresses of the same type configured in the local machine.
When neither `d` nor `s` are provided, the search behaves as if both `d` and `s` were provided.
By default, `mode` is the empty string.

The current standard implementation of this operation may return the following [error messages](#errmsg).

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

### `name [, errmsg] = network.getname (spec)` {#network.getname}

address == address -> nodename, servicename
address == string -> canonicalname
address == number -> servicename

### `socket [, errmsg] = network.socket (type [, domain])` {#network.socket}

This function creates a socket, of the type specified in the string `type`.

The `type` string can be any of the following:

`"datagram"`
:	datagram socket (UDP).
`"connection"`
:	stream socket to initiate connections (TCP).
`"listen"`
:	stream socket to accept connections (TCP).

The `domain` string defines the socket's address domain (or family) and can be any of the following:

`"ipv4"`
:	IPv4 addresses. (the default)
`"ipv6"`
:	IPv6 addresses.

It returns a new socket handle, or, in case of errors, `nil` plus an error message.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"no resources"` (no descriptors available)
- `"access denied"`
- `"no system memory"`

### `type = network.type (value)` {#network.type}

Returns the string `"address"` if `value` is an address structure, or `"socket"` if `value`is a socket handle; otherwise it returns `nil`.

### `type = socket.type` {#socket.type}

The type of `socket`, which can be: `"datagram"` `"connection"`, `"listen"`.
This field is read-only.

### `success [, errmsg] = socket:close ()` {#socket:close}

Closes `socket`.
Note that sockets are automatically closed when their handles are garbage collected, but that takes an unpredictable amount of time to happen. 

In case of success, this function returns `true`.
Otherwise it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"unfulfilled"` (interrupted by signal)
- `"system error"`

### `success [, errmsg] = socket:bind (address)` {#socket:bind}

Binds `socket` to the local address provided as `address`.

In case of success, this function returns `true`.
Otherwise it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"in use"` (socket is connected)
- `"unavailable"` (address is in use)
- `"unreachable"` (address is not local)
- `"no system memory"`

### `address [, errmsg] = socket:getaddress ([site [, address]])` {#socket:getaddress}

Returns the address associated with `socket`, as indicated by `site`, which can be:

`"this"`
:	The socket's address (the default).

`"peer"`
:	The socket's peer address.

If `address` is provided, it is the address structure used to store the result.

In case of errors, it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"closed"` (socket is not connected)
- `"no system memory"`

### `value [, errmsg] = socket:getoption (name)` {#socket:getoption}

Returns the value of option `name` of `socket`, or, in case of errors, `nil` plus an error message.
There available options are:

- `"blocking"`: is `false` for a non-blocking socket, or `true` otherwise.
- `"reuseaddr"`: is `true` when address reuse is allowed for `socket`, or `false`otherwise.
- `"dontroute"`: is `true` when routing facilities are disabled for `socket`, or `false` otherwise.

Sockets of type `"connection" accept the additional options:

- `"linger"`: is the number of seconds that `socket:close` can be delayed when there is pending data to be sent.
- `"keepalive"`: is `true` when periodic transmission of messages is enabled for `socket`, or `false` otherwise.
- `"nodelay"`: is `true` when coalescing of small segments shall be avoided in `socket`, or `false` otherwise.

Sockets type `"datagram"` accept the additional option:

- `"broadcast"`: is `true` when sending of broadcast messages are permited for `socket`, or `false` otherwise.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"access denied"`
- `"no system memory"`

### `success [, errmsg] = socket:setoption (name, value)` {#socket:setoption}

Sets the option `name` for `socket`.
There available options are the same as defined in operation `socket:getoption`.

In case of success, this function returns `true`.
Otherwise it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"in use"` (socket is connected)
- `"too much"` (value too large for option)

### `success [, errmsg] = socket:listen ([backlog])` {#socket:listen}

Starts listening for new connections on socket.
`backlog` is a hint for the underlying system about the suggested number of outstanding connections that shall be kept in the socket's listen queue.
By default `backlog` is 32.

This operation is only available for `listen` sockets.

In case of success, this function returns `true`.
Otherwise it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"access denied"`
- `"no resources"`

### `connection [, errmsg] = socket:accept ([address])` {#socket:accept}

Accepts a new pending connection on `socket`.
An address structure can be provided as `address` to be filled with the address of the accepted connection's peer socket.

This operation is only available for `listen` sockets.

In case of success, this function returns a new `connection` socket for the accepted connection.
Otherwise it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"unfulfilled"`
- `"no resources"`
- `"aborted"` (a connection has been aborted)
- `"no resources"`
- `"no system memory"`

### `success [, errmsg] = socket:connect (address)` {#socket:connect}

Binds `socket` to the peer address provided as `address`.
This operation is not available for `listen` sockets.

In case of success, this function returns `true`.
Otherwise it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"unfulfilled"`
- `"in use"` (address in use)
- `"unavailable"` (address not available)
- `"unreachable"` (address in unreachable)
- `"refused"` (connection refused by peer)
- `"aborted"` (connection reset by peer)
- `"timeout"`
- `"system down"`
- `"no system memory"`

### `sent [, errmsg] = socket:send (data [, i [, j [, address]]])` {#socket:send}

Sends the substring of `data` that starts at `i` and continues until `j`; `i` and `j` can be negative.
If `j` is absent, then it is assumed to be equal to -1 (which is the same as the string length).

This operation is not available for `listen` sockets.
For disconnected datagram sockets, it is necessary to provide `address` with the peer address the data is destined.

In case of success, this function returns the number of bytes from `data` effectivelly sent through `socket`.
Otherwise it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"unfulfilled"`
- `"too much"` (message too large)
- `"closed"` (connection is closed)
- `"aborted"` (connection reset by peer)
- `"access denied"`
- `"unreachable"` (peer is unreachable)
- `"no system memory"`
- `"system down"`
- `"system error"`

### `data [, errmsg] = socket:receive (size [, mode [, address]])` {#socket:receive}

Receives up to `size` bytes from socket `socket`, using the mode specified in the string `mode`.
The mode string can be contain any of the following characters:

- `p`: only peeks at an incoming message, leaving the returned data as unread in `socket`.
- `a`: only returns successfully when all the `size` bytes were received (only for `connection` sockets).

By default, `mode` is the empty string, which disables all options.

This operation is not available for `listen` sockets.
For disconnected datagram sockets, it is possible to provide `address` with an address structure to be filled with the address of the peer socket from which the received data were originated.

In case of success, this function returns the `data` effectivelly received from `socket`.
Otherwise it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"unfulfilled"`
- `"timeout"`
- `"closed"` (connection is closed)
- `"aborted"` (connection reset by peer)
- `"no resources"`
- `"no system memory"`
- `"system down"` (network interface is down)
- `"system error"`

### `success [, errmsg] = socket:shutdown ([mode])` {#socket:shutdown}

Shuts down `socket` connection in one or both directions, as specified in the string `mode`.
The mode string can be any of the following:

- `send`: disables further send operations.
- `receive`: disables further receive operations.
- `both`: disables both send and receive operations. (the default)

This operation is only available for `connection` sockets.

In case of success, this function returns `true`.
Otherwise it returns `nil` plus a string describing the error.

The current standard implementation of this operation may return the following [error messages](#errmsg).

- `"closed"` (socket is not connected)
- `"no resources"`

Error Messages {#errmsg}
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
:	underlying system raised an unexpected error, usually due to poor LOSKI support for the particular platform.

`"unspecified error"`
:	underlying system raised a non-conforming error, usually due to poor platform adherence to standards.

`"unknown error (<number>)"`
:	underlying system raised an illegal error value (indicated in the message) due to wrong LOSKI implementation or build.
