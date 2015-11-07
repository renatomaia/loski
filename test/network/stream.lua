local time = require "time"
local network = require "network"
local tests = require "test.network.utils"

local addr = network.address("0.0.0.0", 80)
for _, kind in ipairs{"datagram", "connection"} do
	local socket = tests.testcreatesocket(kind)

	if tests.IsWindows then
		tests.testerrmsg("address unavailable", socket:connect(addr))
	elseif kind == "connection" then
		tests.testerrmsg("refused", socket:connect(addr))
	else
		assert(socket:connect(addr) == true)
	end

	tests.testoptions(socket, kind, "refused")
	tests.testclose(socket)
end

for _, kind in ipairs{"datagram", "connection"} do
	local socket = tests.testcreatesocket(kind)

	assert(socket:connect(tests.RemoteTCP) == true)
	if kind == "datagram" then
		assert(socket:connect(tests.OtherTCP) == true)
	else
		tests.testerrmsg("connected", socket:connect(tests.OtherTCP))
	end

	tests.testoptions(socket, kind)
	tests.testclose(socket)
end

for _, kind in ipairs{"datagram", "connection"} do
	local socket = tests.testcreatesocket(kind)
	assert(socket:setoption("blocking", false) == true)

	if kind == "datagram" then
		assert(socket:connect(tests.RemoteTCP) == true)
	else
		tests.testerrmsg("connected",
			tests.tcall(true, socket.connect, socket, tests.RemoteTCP))
	end

	tests.testclose(socket)
end
