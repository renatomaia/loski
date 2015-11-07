Index
=====

- [`network`](#network)
- [`network.address`](#network.address)
- [`network.getname`](#network.getname)
- [`network.resolve`](#network.resolve)
- [`network.socket`](#network.socket)
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

Contents
========

Network Communication {#network}
---------------------

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

### `next [, errmsg] = network.resolve (name [, servname [, what]])` {#network.resolve}

`what`:

- `l` local only
- `4` ipv4
- `6` ipv6
- `m` mapped ipv4
- `d` datagram
- `s` stream

	-- [address, socktype, more =] next([address])

	-- get first result only
	addr, scktype = dns.resolve("www.google.com", "http")()

	-- collect all results
	list = {}
	for addr, scktype in dns.resolve("www.google.com", "http") do
		list[#list+1] = { addr, scktype }
	end

	-- iterating over the result data (using a single address object)
	next = dns.resolve("www.google.com", "http")
	for addr, scktype in next, address.create() do
		print(addr, scktype)
	end

	-- filling existing addreses object with the results
	next = dns.resolve("www.google.com", "http")
	repeat until not select(3, next(getSomeAddressObject()))

### `name [, errmsg] = network.getname (spec)` {#network.getname}

address == address -> nodename, servicename
address == string -> canonicalname
address == number -> servicename

### `socket [, errmsg] = network.socket (type [, domain])` {#network.socket}
### `succ [, errmsg] = socket:close ()` {#socket:close}
### `succ [, errmsg] = socket:bind (address)` {#socket:bind}
### `address [, errmsg] = socket:getaddress (address [, site])` {#socket:getaddress}
### `value [, errmsg] = socket:getoption (name)` {#socket:getoption}
### `succ [, errmsg] = socket:setoption (name, value)` {#socket:setoption}

### `conn [, errmsg] = socket:accept ([address])` {#socket:accept}
### `succ [, errmsg] = socket:listen ([backlog])` {#socket:listen}

### `succ [, errmsg] = socket:connect (address)` {#socket:connect}
### `sent [, errmsg] = socket:send (data [, i [, j [, address]]])` {#socket:send}
### `data [, errmsg] = socket:receive (size [, address])` {#socket:receive}

### `succ [, errmsg] = socket:shutdown([mode])` {#socket:shutdown}
