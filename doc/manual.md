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

### `address.create ([data [, port [, type]]])` {#address.create}

Returns a new address object initialized with the data provided by the parameters.
If no parameters are provided the created address contains null data.
When `type` is:

`"uri"`
:	`data` must be a string describing the address as inside a URI, like `"192.168.0.20:80"` (IPv4) or `[::1]:80` (IPv6).
	`port` shall be `nil` or omitted.
	This is the default value for `type`.

`"literal"`
:	`data` must be a literal representation of an IP address, like `"192.168.0.20"` (IPv4) or `::1` (IPv6).
	`port` must be provided with the port number of the address.

`"bytes"`
:	`data` is a byte representation of an IP address, like `"\192\168\0\20"` (IPv4) or `"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1"` (IPv6).
	`port` must be provided with the port number of the address.

The returned object provides the followin fields, which can modified to change the address contents:

`type`
:	is either the string `"ipv4"` or `"ipv6"` to indicate the address is a IPv4 or IPv6 address, respectively.
	When the value of this field changes, the whole address data is set to null values.

`literal`
:	is the literal representation of the address, like `"192.168.0.20"` (IPv4) or `::1` (IPv6).

`bytes`
:	is the string with the bytes of the address, like `"\192\168\0\20"` (IPv4) or `"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1"` (IPv6).

`port`
:	is the port number of the address.

Moreover, you can pass the object to the standard function `tostring` to obtain the address as a string inside a URI, like `"192.168.0.20:80"` (IPv4) or `[::1]:80` (IPv6).

### `address.type (addr)` {#address.type}

Returns the strings `"ipv4"` or `"ipv6"` to indicate address `addr` is either a IPv4 or IPv6 address.
Moreover, it returns `nil` if `addr` is not a valid address.
