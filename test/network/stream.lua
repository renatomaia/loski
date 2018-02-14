local time = require "time"
local network = require "network"
local tests = require "test.network.utils"

local addr = network.address("ipv4", "0.0.0.0", 80)
for _, kind in ipairs{"datagram", "stream"} do
	local socket = tests.testcreatesocket(kind)

	if tests.IsWindows then
		tests.testerrmsg("unavailable", socket:connect(addr))
	elseif kind == "stream" then
		tests.testerrmsg("refused", socket:connect(addr))
	else
		assert(socket:connect(addr) == true)
	end

	tests.testclose(socket)
end

for _, kind in ipairs{"datagram", "stream"} do
	local socket = tests.testcreatesocket(kind)

	assert(socket:connect(tests.RemoteTCP) == true)
	assert(socket:getaddress("peer") == tests.RemoteTCP)
	if kind == "datagram" then
		assert(socket:connect(tests.OtherTCP) == true)
		assert(socket:getaddress("peer") == tests.OtherTCP)
	else
		tests.testerrmsg("in use", socket:connect(tests.OtherTCP))
		assert(socket:getaddress("peer") == tests.RemoteTCP)
	end

	tests.testoptions(socket, kind)
	tests.testclose(socket)
end

for _, kind in ipairs{"datagram", "stream"} do
	local socket = tests.testcreatesocket(kind)
	tests.testerror("unknown mode char (got 'l')",
	                socket.receive, socket, 5, "all")
	tests.testclose(socket)
end

for _, kind in ipairs{"datagram", "stream"} do
	local socket = tests.testcreatesocket(kind)
	assert(socket:setoption("blocking", false) == true)

	if kind == "datagram" then
		assert(socket:connect(tests.RemoteTCP) == true)
	else
		tests.tcall(true, socket.connect, socket, tests.RemoteTCP)
	end

	tests.testclose(socket)
end
