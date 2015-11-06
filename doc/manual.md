Index
=====

- [`network.address`](#network.address)
- [`address.create`](#address.create)
- [`address.type`](#address.type)

Contents
========

Network Address Manipulation {#network.address}
----------------------------

This library provides generic functions for manipulation of strings containing raw network addresses.

### `address.create ([data [, port [, mode]]])` {#address.create}

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

### `address.type (addr)` {#address.type}

Returns the strings `"ipv4"` or `"ipv6"` to indicate address `addr` is either a IPv4 or IPv6 address.
Moreover, it returns `nil` if `addr` is not a valid address.
