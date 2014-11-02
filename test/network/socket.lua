local network = require "network"
local tests = require "test.network.utils"

for _, kind in ipairs{"datagram", "connection", "listen"} do
	local cases = {
		["address unavailable"] = {host="255.255.255.255", port=tests.FreePort},
		["access denied"] = not tests.IsWindows and {host=tests.LocalHost, port=tests.DeniedPort} or nil,
	}
	if kind ~= "datagram" then
		cases["address used"] = {host=tests.LocalHost, port=tests.UsedPort}
	end
	for expected, address in pairs(cases) do
		local socket = tests.testcreatesocket(kind)
		tests.testoptions(socket, kind)

		-- succ [, errmsg] = socket:bind(host, port)
		tests.testerrmsg(expected, socket:bind(address.host, address.port))

		tests.testoptions(socket, kind)
		tests.testclose(socket)
	end
end

for _, kind in ipairs{"datagram", "connection", "listen"} do
	local socket = tests.testcreatesocket(kind)
	tests.testoptions(socket, kind)

	-- succ [, errmsg] = socket:bind(host, port)
	assert(socket:bind(tests.LocalHost, tests.FreePort) == true)
	tests.testerrmsg("unsupported", socket:bind(tests.LocalHost, tests.LocalPort))

	-- host, port = socket:getaddress([site])
	local host, port = socket:getaddress()
	assert(host == tests.LocalHost)
	assert(port == tests.FreePort)
	local host, port = socket:getaddress("local")
	assert(host == tests.LocalHost)
	assert(port == tests.FreePort)
	tests.testerrmsg("disconnected", socket:getaddress("remote"))

	tests.testoptions(socket, kind)
	tests.testclose(socket)
end

for _, kind in ipairs{"datagram", "connection", "listen"} do
	local socket = tests.testcreatesocket(kind)
	assert(socket:setoption("reuseaddr", true) == true)
	assert(socket:bind(tests.LocalHost, tests.UsedPort) == true)
	tests.testclose(socket)
end

