Index
=====

- [`network.address`](#network.address)
- [`address.create`](#address.create)
- [`address.type`](#address.type)
- [`address.uri`](#address.uri)
- [`address.rawaddr`](#address.rawaddr)
- [`address.addr`](#address.host)
- [`address.port`](#address.port)

Contents
========

Network Address Manipulation {#network.address}
----------------------------

This library provides generic functions for manipulation of strings containing raw network addresses.

### `address.create (type, addr [, port])` {#address.create}

Returns an object representing the network address specified by the provided paramenters.
When `type` is:

`"ipv4"`
:	`addr` is a byte representation of a IPv4 address, like `"\192\168\0\20"`.
	`port` must be provided with the port number of the address.

`"inet"`
:	`addr` is a textual representation of a IPv4 address, like `"192.168.0.20"`.
	`port` must be provided with the port number of the address.

`"ipv6"`
:	`addr` is a byte representation of a IPv6 address, like `"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1"`.
	`port` must be provided with the port number of the address.

`"inet6"`
:	`addr` is a textual representation of a IPv6 address, like `"::1"`.
	`port` must be provided with the port number of the address.

`"uri"`
:	`addr` is string describing the address as inside a URI, like `"192.168.0.20:80"` (IPv4) or `[::1]:80` (IPv6).
	`port` is ignored and can be omitted.

### `address.type (addr)` {#address.type}

Return a string indicating the type of the address or `nil` if `addr` is not a valid address.
The possible results are:

`"ipv4"`
:	Indicates a IPv4 address.

`"ipv6"`
:	Indicates a IPv6 address.

### `address.uri (addr)` {#address.uri}

