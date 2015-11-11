local network = require "network"
local utils = require "test.utils"

do
	assert(network.type(nil) == nil)
	assert(network.type(false) == nil)
	assert(network.type(true) == nil)
	assert(network.type(3232235776) == nil)
	assert(network.type("192.168.0.1:8080") == nil)
	assert(network.type(table) == nil)
	assert(network.type(print) == nil)
	assert(network.type(coroutine.running()) == nil)
	assert(network.type(io.stdout) == nil)
	assert(network.type(network.address()) == "address")
	assert(network.type(network.socket("listen")) == "socket")
	assert(network.type(network.socket("connection")) == "socket")
	assert(network.type(network.socket("datagram")) == "socket")
end

do
	local a = network.address()

	assert(tostring(a) == "0.0.0.0:0")
	assert(a.type == "ipv4")
	assert(a.port == 0)
	assert(a.literal == "0.0.0.0")
	assert(a.binary == "\000\000\000\000")
	assert(a == network.address())
end

local cases = {
	ipv4 = {
		port = 8080,
		literal = "192.168.0.1",
		binary = "\192\168\000\001",
		uri = "192.168.0.1:8080",
		changes = {
			type = "ipv4",
			port = 54321,
			literal = "127.0.0.1",
			binary = "byte",
		},
		equivalents = {
			["10.20.30.40"] = "\10\20\30\40",
			["40.30.20.10"] = "\40\30\20\10",
		},
	},
	ipv6 = {
		port = 8888,
		literal = "::ffff:192.168.0.1",
		binary = "\0\0\0\0\0\0\0\0\0\0\255\255\192\168\000\001",
		uri = "[::ffff:192.168.0.1]:8888",
		changes = {
			type = "ipv6",
			port = 12345,
			literal = "::1",
			binary = "bytebytebytebyte",
		},
		equivalents = {
			["1::f"] =
				"\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\15",
			["1:203:405:607:809:a0b:c0d:e0f"] =
				"\0\1\2\3\4\5\6\7\8\9\10\11\12\13\14\15",
		},
	},
}
for type, case in pairs(cases) do
	local addr = network.address(case.uri)

	local function checkaddr(a)
		assert(network.type(a) == "address")
		assert(tostring(a) == case.uri)
		assert(a.type == type)
		assert(a.port == case.port)
		assert(a.literal == case.literal)
		assert(a.binary == case.binary)
		assert(a == addr)
	end

	checkaddr(addr)
	checkaddr(network.address(case.literal, case.port))
	checkaddr(network.address(case.literal, case.port, "t"))
	checkaddr(network.address(case.binary, case.port, "b"))

	for field, newval in pairs(case.changes) do
		local oldval = addr[field]
		addr[field] = newval
		assert(addr[field] == newval)
		addr[field] = oldval
		checkaddr(addr)
		case.changes[field] = nil
	end
	assert(next(case.changes) == nil)

	for literal, binary in pairs(case.equivalents) do
		addr.literal = literal
		assert(addr.binary == binary)
	end
	for literal, binary in pairs(case.equivalents) do
		addr.binary = binary
		assert(addr.literal == literal)
	end
end

do
	local a = network.address()
	utils.testerror("bad argument #2 to '__index' (invalid option 'wrongfield')",
		function () return a.wrongfield end)
	utils.testerror("bad argument #2 to '__newindex' (invalid option 'wrongfield')",
		function () a.wrongfield = true end)
	utils.testerror("bad argument #3 to '__newindex' (invalid option 'unix')",
		function () a.type = "unix" end)
end

do
	utils.testerror("bad argument #1 to '?' (string expected, got boolean)",
		network.address, true)
	utils.testerror("bad argument #1 to '?' (string expected, got boolean)",
		network.address, true, 8080)
	utils.testerror("bad argument #1 to '?' (string expected, got boolean)",
		network.address, true, 8080, "t")
	utils.testerror("bad argument #1 to '?' (string expected, got boolean)",
		network.address, true, 8080, "b")
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		network.address, nil)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		network.address, nil, nil)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		network.address, nil, nil, nil)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		network.address, nil, 8080)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		network.address, nil, 8080, "t")
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		network.address, nil, 8080, "b")
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		network.address, nil, nil)
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		network.address, nil, nil, "t")
	utils.testerror("bad argument #1 to '?' (string expected, got nil)",
		network.address, nil, nil, "b")
	utils.testerror("bad argument #2 to '?' (number expected, got nil)",
		network.address, "192.168.0.1:8080", nil)
	utils.testerror("bad argument #2 to '?' (number expected, got nil)",
		network.address, "192.168.0.1", nil, "t")
	utils.testerror("bad argument #2 to '?' (number expected, got nil)",
		network.address, "\192\168\0\1", nil, "b")
	utils.testerror("bad argument #2 to '?' (number expected, got string)",
		network.address, "192.168.0.1", "port")
	utils.testerror("bad argument #2 to '?' (number expected, got string)",
		network.address, "192.168.0.1", "port", "t")
	utils.testerror("bad argument #2 to '?' (number expected, got string)",
		network.address, "192.168.0.1", "port", "b")
	utils.testerror("bad argument #3 to '?' (invalid mode)",
		network.address, 3232235776, 8080, "n")

	utils.testerror("bad argument #1 to '?' (invalid URI format)",
		network.address, "192.168.0.1")
	utils.testerror("invalid operation",
		network.address, "localhost:8080")
	utils.testerror("invalid operation",
		network.address, "291.168.0.1:8080")
	utils.testerror("bad argument #1 to '?' (invalid port)",
		network.address, "192.168.0.1:65536")
	utils.testerror("bad argument #1 to '?' (invalid port)",
		network.address, "192.168.0.1:-8080")
	utils.testerror("bad argument #1 to '?' (invalid port)",
		network.address, "192.168.0.1:0x1f90")

	utils.testerror("invalid operation",
		network.address, "localhost", 8080, "t")
	utils.testerror("invalid operation",
		network.address, "291.168.0.1", 8080, "t")

	utils.testerror("bad argument #2 to '?' (invalid port)",
		network.address, "192.168.0.1", 65536, "t")
	utils.testerror("bad argument #2 to '?' (invalid port)",
		network.address, "192.168.0.1", -1, "t")
	utils.testerror("bad argument #2 to '?' (invalid port)",
		network.address, "\192\168\000\001", 65536, "b")
	utils.testerror("bad argument #2 to '?' (invalid port)",
		network.address, "\192\168\000\001", -1, "b")
end
