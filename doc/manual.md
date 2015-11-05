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

### `address.create (data [, port [, type]])` {#address.create}

Returns an object representing the network address from the provided paramenters.
When `type` is:

`"uri"`
:	`addr` is string describing the address as inside a URI, like `"192.168.0.20:80"` (IPv4) or `[::1]:80` (IPv6).
	This is the default value for `type`.
	`port` shall be `nil`or omitted.

`"literal"`
:	`addr` is a textual representation of a IP address, like `"192.168.0.20"` (IPv4) or `::1` (IPv6).
	`port` must be provided with the port number of the address.

`"bytes"`
:	`addr` is a byte representation of an IP address, like `"\192\168\0\20"` (IPv4) or `"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1"` (IPv6).
	`port` must be provided with the port number of the address.

The returned object provides the followin fields:

`type`
:	Returns the strings `"ipv4"` or `"ipv6"` to indicate address `addr` is either a IPv4 or IPv6 address.
	Moreover, it returns `nil` if `addr` is not a valid address.

`literal`
:	Returns a string with the literal representation of the address `addr`, like `"192.168.0.20"` (IPv4) or `::1` (IPv6).

`bytes`
:	Returns a string with the bytes of the address `addr`, like `"\192\168\0\20"` (IPv4) or `"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1"` (IPv6).

`port`
:	Returns a port number of the address `addr`.

Moreover, you can pass the object to the standard function `tostring` to obtain the address as a string inside a URI, like `"192.168.0.20:80"` (IPv4) or `[::1]:80` (IPv6).

### `address.type (addr)` {#address.type}

Returns the strings `"ipv4"` or `"ipv6"` to indicate address `addr` is either a IPv4 or IPv6 address.
Moreover, it returns `nil` if `addr` is not a valid address.
