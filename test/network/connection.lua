local time = require "time"
local network = require "network"
local tests = require "test.network.utils"

local packsize = 64
local packdata = string.rep(" ", packsize)
local packhuge = string.rep(packdata, 2^15)
local packfrmt = "%"..packsize.."s"
local packback = string.rep("\255", packsize)
local replycount = 3
local remotecode = [[
	local time = require "time"
	local network = require "network"

	local addr = network.address("0.0.0.0", ]]..tests.LocalAddress.port..[[)
	local port = assert(network.socket("listen"))
	assert(port:setoption("reuseaddr", true) == true)
	assert(port:bind(addr) == true)
	assert(port:listen() == true)
	local conn = assert(port:accept())
	assert(port:close())
	local packsize = ]]..packsize..[[
	local packdata = string.rep("\255", packsize)
	local count = 0
	local message
	while message == nil do
		message = conn:receive(packsize)
		assert(#message <= packsize)
		count = count + #message
		message = string.match(message, "[%S]+")
	end
	assert(tonumber(message) == count-packsize)
	for i=1, ]]..replycount..[[ do
		time.sleep(.9)
		assert(conn:send(packdata) == packsize)
	end
	assert(conn:close())
]]

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("connection")

	assert(socket:connect(tests.LocalAddress) == true)
	for i = 1, 3 do
		assert(socket:send(packdata) == packsize)
	end
	assert(socket:send(string.format(packfrmt, 3*packsize)) == packsize)
	local remaining = replycount*packsize
	while remaining > 0 do
		assert(socket:receive(remaining) == packback)
		remaining = remaining - packsize
	end

	tests.testclose(socket)
end

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("connection")
	assert(socket:setoption("blocking", false) == true)

	tests.testerrmsg("connected",
		tests.tcall(true, socket.connect, socket, tests.LocalAddress))

	local sent = 0
	while true do
		local res, errmsg = socket:send(packhuge)
		if res ~= nil then
			assert(res <= #packhuge)
			sent = sent + res
		else
			assert(errmsg == "unfulfilled")
			time.sleep(.1)
			break
		end
	end
	tests.testerrmsg("unfulfilled", socket:receive(packsize))
	assert(socket:send(string.format(packfrmt, sent)) == packsize)
	local remaining = replycount*packsize
	local count = 0
	while count < remaining do
		local received = tests.tcall(count==0, socket.receive, socket, remaining-count)
		assert(string.find(received, "[^\255]") == nil)
		count = count + #received
	end
	tests.testerrmsg("disconnected", tests.tcall(true, socket.receive, socket, 1))

	tests.testclose(socket)
end
