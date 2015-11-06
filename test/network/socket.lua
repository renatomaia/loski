local network = require "network"
local address = require "network.address"
local tests = require "test.network.utils"

for _, kind in ipairs{"datagram", "connection", "listen"} do
	local cases = {
		["address unavailable"] = address.create("255.255.255.255",
		                                         tests.FreeAddress.port)
	}
	if not tests.IsWindows then
		cases["access denied"] = tests.DeniedAddress
	end
	if kind ~= "datagram" then
		cases["address used"] = tests.UsedAddress
	end
	for expected, addr in pairs(cases) do
		local socket = tests.testcreatesocket(kind)
		tests.testoptions(socket, kind)

		-- succ [, errmsg] = socket:bind(address)
		tests.testerrmsg(expected, socket:bind(addr))

		tests.testoptions(socket, kind)
		tests.testclose(socket)
	end
end

for _, kind in ipairs{"datagram", "connection", "listen"} do
	local socket = tests.testcreatesocket(kind)
	tests.testoptions(socket, kind)

	-- succ [, errmsg] = socket:bind(address)
	assert(socket:bind(tests.FreeAddress) == true)
	tests.testerrmsg("unsupported", socket:bind(tests.LocalAddress))

	-- address [, errmsg] = socket:getaddress(address, [site])
	local addr = socket:getaddress(address.create())
	assert(addr == tests.FreeAddress)
	local addr = socket:getaddress(address.create(), "local")
	assert(addr == tests.FreeAddress)
	tests.testerrmsg("disconnected", socket:getaddress(addr, "remote"))

	tests.testoptions(socket, kind)
	tests.testclose(socket)
end

-- TODO: Find out the correct way to test is 'reuseaddr' works.
--       The following only works is UsedAddress.port is binded using different
--       hosts.
--for _, kind in ipairs{"datagram", "connection", "listen"} do
--	local socket = tests.testcreatesocket(kind)
--	assert(socket:setoption("reuseaddr", true) == true)
--	assert(socket:bind(tests.UsedAddress) == true)
--	tests.testclose(socket)
--end
