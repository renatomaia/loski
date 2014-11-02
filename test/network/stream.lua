local time = require "time"
local network = require "network"
local tests = require "test.network.utils"

for _, kind in ipairs{"datagram", "connection"} do
	local socket = tests.testcreatesocket(kind)

	if tests.IsWindows then
		tests.testerrmsg("address unavailable", socket:connect("0.0.0.0", 80))
	elseif kind == "connection" then
		tests.testerrmsg("refused", socket:connect("0.0.0.0", 80))
	else
		assert(socket:connect("0.0.0.0", 80) == true)
	end

	tests.testoptions(socket, kind, "refused")
	tests.testclose(socket)
end

for _, kind in ipairs{"datagram", "connection"} do
	local socket = tests.testcreatesocket(kind)

	assert(socket:connect(tests.RemoteTCP.host, tests.RemoteTCP.port) == true)
	if kind == "datagram" then
		assert(socket:connect(tests.OtherTCP.host, tests.OtherTCP.port) == true)
	else
		tests.testerrmsg("connected", socket:connect(tests.OtherTCP.host, tests.OtherTCP.port))
	end

	tests.testoptions(socket, kind)
	tests.testclose(socket)
end

for _, kind in ipairs{"datagram", "connection"} do
	local socket = tests.testcreatesocket(kind)
	assert(socket:setoption("blocking", false) == true)

	if kind == "datagram" then
		assert(socket:connect(tests.RemoteTCP.host, tests.RemoteTCP.port) == true)
	else
		tests.testerrmsg("connected",
			tests.tcall(true, socket.connect, socket, tests.RemoteTCP.host, tests.RemoteTCP.port))
	end

	tests.testclose(socket)
end
