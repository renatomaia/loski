local address = require "network.address"
local utils = require "test.utils"

do
	assert(address.type(nil) == nil)
	assert(address.type(false) == nil)
	assert(address.type(true) == nil)
	assert(address.type(3232235776) == nil)
	assert(address.type("192.168.0.1:8080") == nil)
	assert(address.type(table) == nil)
	assert(address.type(print) == nil)
	assert(address.type(coroutine.running()) == nil)
	assert(address.type(io.stdout) == nil)
end

do
	local a = address.create()

	assert(address.type(a) == "ipv4")
	assert(tostring(a) == "0.0.0.0:0")
	assert(a.type == "ipv4")
	assert(a.port == 0)
	assert(a.literal == "0.0.0.0")
	assert(a.binary == "\000\000\000\000")
	assert(a == address.create())
end

do
	local addr = address.create("192.168.0.1:8080")

	local function checkaddr(a)
		assert(address.type(a) == "ipv4")
		assert(tostring(a) == "192.168.0.1:8080")
		assert(a.type == "ipv4")
		assert(a.port == 8080)
		assert(a.literal == "192.168.0.1")
		assert(a.binary == "\192\168\000\001")
		assert(a == addr)
	end

	checkaddr(addr)
	checkaddr(address.create("192.168.0.1", 8080))
	checkaddr(address.create("192.168.0.1", 8080, "t"))
	checkaddr(address.create("\192\168\000\001", 8080, "b"))

	local othervals = {
		type = "ipv4",
		port = 54321,
		literal = "127.0.0.1",
		binary = "byte",
	}
	for field, newval in pairs(othervals) do
		local oldval = addr[field]
		addr[field] = newval
		assert(addr[field] == newval)
		addr[field] = oldval
		checkaddr(addr)
		othervals[field] = nil
	end
	assert(next(othervals) == nil)

	addr.literal = "10.20.30.40"
	assert(addr.binary == "\10\20\30\40")
	addr.binary = "\40\30\20\10"
	assert(addr.literal == "40.30.20.10")
end

do
	local a = address.create()
	utils.testerror("bad argument #2 to '__index' (invalid option 'wrongfield')",
		function () return a.wrongfield end)
	utils.testerror("bad argument #2 to '__newindex' (invalid option 'wrongfield')",
		function () a.wrongfield = true end)
	utils.testerror("bad argument #3 to '__newindex' (invalid option 'unix')",
		function () a.type = "unix" end)
	utils.testerror("bad argument #3 to '__newindex' (unsupported address)",
		function () a.type = "ipv6" end)
end

do
	utils.testerror("bad argument #1 to '?' (string expected, got boolean)",
		address.create, true)
	utils.testerror("bad argument #1 to '?' (string expected, got boolean)",
		address.create, true, 8080)
	utils.testerror("bad argument #1 to '?' (string expected, got boolean)",
		address.create, true, 8080, "t")
	utils.testerror("bad argument #1 to '?' (string expected, got boolean)",
		address.create, true, 8080, "b")
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		address.create, nil)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		address.create, nil, nil)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		address.create, nil, nil, nil)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		address.create, nil, 8080)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		address.create, nil, 8080, "t")
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		address.create, nil, 8080, "b")
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		address.create, nil, nil)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		address.create, nil, nil, "t")
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		address.create, nil, nil, "b")
	utils.testerror("bad argument #2 to '?' (number expected, got nil)",
		address.create, "192.168.0.1:8080", nil)
	utils.testerror("bad argument #2 to '?' (number expected, got nil)",
		address.create, "192.168.0.1", nil, "t")
	utils.testerror("bad argument #2 to '?' (number expected, got nil)",
		address.create, "\192\168\0\1", nil, "b")
	utils.testerror("bad argument #2 to '?' (number expected, got string)",
		address.create, "192.168.0.1", "port")
	utils.testerror("bad argument #2 to '?' (number expected, got string)",
		address.create, "192.168.0.1", "port", "t")
	utils.testerror("bad argument #2 to '?' (number expected, got string)",
		address.create, "192.168.0.1", "port", "b")
	utils.testerror("bad argument #3 to '?' (invalid mode)",
		address.create, 3232235776, 8080, "n")

	utils.testerror("bad argument #1 to '?' (invalid URI format)",
		address.create, "192.168.0.1")
	utils.testerror("bad argument #1 to '?' (invalid literal address)",
		address.create, "localhost:8080")
	utils.testerror("bad argument #1 to '?' (invalid literal address)",
		address.create, "291.168.0.1:8080")
	utils.testerror("bad argument #1 to '?' (invalid port)",
		address.create, "192.168.0.1:65536")
	utils.testerror("bad argument #1 to '?' (invalid port)",
		address.create, "192.168.0.1:-8080")
	utils.testerror("bad argument #1 to '?' (invalid port)",
		address.create, "192.168.0.1:0x1f90")

	utils.testerror("bad argument #1 to '?' (invalid literal address)",
		address.create, "localhost", 8080, "t")
	utils.testerror("bad argument #1 to '?' (invalid literal address)",
		address.create, "291.168.0.1", 8080, "t")

	utils.testerror("bad argument #2 to '?' (invalid port)",
		address.create, "192.168.0.1", 65536, "t")
	utils.testerror("bad argument #2 to '?' (invalid port)",
		address.create, "192.168.0.1", -1, "t")
	utils.testerror("bad argument #2 to '?' (invalid port)",
		address.create, "\192\168\000\001", 65536, "b")
	utils.testerror("bad argument #2 to '?' (invalid port)",
		address.create, "\192\168\000\001", -1, "b")

	-- no IPv6 support
	utils.testerror("bad argument #1 to '?' (unsupported address)",
		address.create, "[::1]:8080")
	utils.testerror("bad argument #1 to '?' (unsupported address)",
		address.create, "::1", 8080, "t")
	utils.testerror("bad argument #1 to '?' (unsupported address)",
		address.create, string.rep("\0", 15).."\1", 8080, "b")
end
