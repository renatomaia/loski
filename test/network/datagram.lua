local time = require "time"
local network = require "network"
local memory = require "memory"
local tests = require "test.network.utils"

local packsize = 64
local data = string.rep("X", packsize)
local final = string.rep("X", packsize-1).."0"
local buffer = memory.create(3*packsize)
local remotecode = [[
	local time = require "time"
	local network = require "network"
	local memory = require "memory"

	local addr = network.address("ipv4", "0.0.0.0", ]]..tests.LocalAddress.port..[[)
	local socket = assert(network.socket("datagram"))
	assert(socket:setoption("reuseaddr", true) == true)
	assert(socket:bind(addr) == true)
	local packsize = ]]..packsize..[[
	local all = {}
	local size = {}
	local i = 0
	repeat
		i = i+1
		all[i] = memory.create(packsize)
		size[i] = assert(socket:receive(all[i], 1, -1, nil, addr))
		assert(addr.literal == "]]..tests.FreeAddress.literal..[[")
		assert(addr.port == ]]..tests.FreeAddress.port..[[)
	until all[i]:get(size[i]) == string.byte("0")
	for index, data in ipairs(all) do
		time.sleep(.9)
		assert(socket:send(data, 1, size[index], addr) == size[index])
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
	local index = 1
	while index <= #buffer do
		local size = assert(socket:receive(buffer, index))
		assert(size == packsize)
		assert(buffer:unpack("c"..size, index) == data)
		index = index + size
	end
	tests.testreceive(socket, final)

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
	local index = 1
	while index <= #buffer do
		local addr = network.address("ipv4")
		local size = assert(socket:receive(buffer, index, -1, nil, addr))
		assert(size == packsize)
		assert(buffer:unpack("c"..size, index) == data)
		index = index + size
	end
	local addr = network.address("ipv4")
	tests.testreceive(socket, final, nil, addr)
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
	tests.testerrmsg("unfulfilled", socket:receive(buffer))
	assert(socket:send(final) == packsize)
	local addr = network.address("ipv4")
	local index = 1
	while index <= #buffer do
		local size = tests.tcall(true, socket.receive, socket, buffer, index, -1, nil, addr)
		assert(size == packsize)
		assert(buffer:unpack("c"..size, index) == data)
		assert(addr == tests.LocalAddress)
		index = index + size
	end
	tests.tcall(true, socket.receive, socket, buffer, 1, packsize)
	assert(memory.diff(buffer, final) == packsize+1)

	tests.testclose(socket)
end
