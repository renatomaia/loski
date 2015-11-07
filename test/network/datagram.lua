local time = require "time"
local network = require "network"
local tests = require "test.network.utils"

local packsize = 64
local data = string.rep("X", packsize)
local final = string.rep("X", packsize-1).."0"
local remotecode = [[
	local time = require "time"
	local network = require "network"

	local addr = network.address("0.0.0.0", ]]..tests.LocalAddress.port..[[)
	local socket = assert(network.socket("datagram"))
	assert(socket:setoption("reuseaddr", true) == true)
	assert(socket:bind(addr) == true)
	local packsize = ]]..packsize..[[
	local all = {}
	local i = 0
	repeat
		i = i+1
		all[i] = assert(socket:receive(packsize, addr))
		assert(addr.literal == "]]..tests.FreeAddress.literal..[[")
		assert(addr.port == ]]..tests.FreeAddress.port..[[)
	until string.find(all[i], "0", nil, "noregex")
	for _, data in ipairs(all) do
		time.sleep(.9)
		assert(socket:send(data, 1, -1, addr) == #data)
	end
	assert(socket:close())
]]

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("datagram")

	assert(socket:bind(tests.FreeAddress) == true)
	assert(socket:connect(tests.LocalAddress) == true)
	for i = 1, 3 do
		assert(socket:send(data) == packsize)
	end
	assert(socket:send(final) == packsize)
	local remaining = 3*packsize
	while remaining > 0 do
		assert(socket:receive(remaining) == data)
		remaining = remaining - #data
	end
	assert(socket:receive(packsize) == final)

	tests.testclose(socket)
end

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("datagram")

	assert(socket:bind(tests.FreeAddress) == true)
	for i = 1, 3 do
		assert(socket:send(data, 1, -1, tests.LocalAddress) == packsize)
	end
	assert(socket:send(final, 1, -1, tests.LocalAddress) == packsize)
	local remaining = 3*packsize
	while remaining > 0 do
		local addr = network.address()
		local received = socket:receive(remaining, addr)
		assert(received == data)
		assert(addr == tests.LocalAddress)
		remaining = remaining - #data
	end
	local addr = network.address()
	local received = socket:receive(packsize, addr)
	assert(received == final)
	assert(addr == tests.LocalAddress)

	tests.testclose(socket)
end

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("datagram")
	assert(socket:setoption("blocking", false) == true)

	assert(socket:bind(tests.FreeAddress) == true)
	assert(socket:connect(tests.LocalAddress) == true)
	for i = 1, 3 do
		assert(socket:send(data) == packsize)
	end
	tests.testerrmsg("unfulfilled", socket:receive(packsize))
	assert(socket:send(final) == packsize)
	local addr = network.address()
	local remaining = 3*packsize
	while remaining > 0 do
		local received = tests.tcall(true, socket.receive, socket, remaining, addr)
		assert(received == data)
		assert(addr == tests.LocalAddress)
		remaining = remaining - #data
	end
	assert(tests.tcall(true, socket.receive, socket, packsize) == final)

	tests.testclose(socket)
end
