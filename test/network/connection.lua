local time = require "time"
local network = require "network"
local memory = require "memory"
local tests = require "test.network.utils"

local packsize = 64
local packdata = string.rep(" ", packsize)
local packhuge = string.rep(packdata, 2^15)
local packfrmt = "%"..(packsize-1).."s\0"
local packback = string.rep("\255", packsize)
local replycount = 3
local remotecode = [[
	local time = require "time"
	local network = require "network"
	local memory = require "memory"

	local addr = network.address("ipv4", "0.0.0.0", ]]..tests.LocalAddress.port..[[)
	local port = assert(network.socket("listen"))
	assert(port:setoption("reuseaddr", true) == true)
	assert(port:bind(addr) == true)
	assert(port:listen() == true)
	local conn = assert(port:accept())
	assert(port:close())
	local packsize = ]]..packsize..[[
	local packdata = string.rep("\255", packsize)
	local packbuffer = memory.create(packsize)
	local count = 0
	local data = ""
	while true do
		local size = assert(conn:receive(packbuffer))
		assert(size <= packsize)
		count = count + size
		local message = packbuffer:unpack(1, "c"..size)
		data = data..(string.match(message, "[%S]+") or "")
		if string.find(message, "\0", 1, true) ~= nil then break end
	end
	assert(tonumber(data:sub(1, -2)) == count-packsize)
	for i=1, ]]..replycount..[[ do
		time.sleep(.5)
		assert(conn:send(packdata) == packsize)
	end
	assert(conn:close())
]]

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("stream")

	assert(socket:connect(tests.LocalAddress) == true)
	for i = 1, 3 do
		assert(socket:send(packdata) == packsize)
	end
	assert(socket:send(string.format(packfrmt, 3*packsize)) == packsize)
	local buffer = memory.create(replycount*packsize)
	local index = 1
	while index <= #buffer do
		assert(socket:receive(buffer, index) == packsize)
		assert(buffer:unpack(index, "c"..packsize) == packback)
		index = index + packsize
	end

	tests.testclose(socket)
end

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("stream")
	assert(socket:setoption("blocking", false) == true)

	tests.tcall(true, socket.connect, socket, tests.LocalAddress)

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
	local buffer = memory.create(replycount*packsize)
	local index = 1
	tests.testerrmsg("unfulfilled", socket:receive(buffer))
	assert(socket:send(string.format(packfrmt, sent)) == packsize)
	while index <= #buffer do
		local received = assert(tests.tcall(count==0, socket.receive, socket,
		                                    buffer, index))
		for i = index, index+received-1 do
			assert(buffer:get(i) == 255)
		end
		index = index + received
	end
	tests.testerrmsg("closed", tests.tcall(true, socket.receive, socket, buffer, 1, 1))

	tests.testclose(socket)
end
