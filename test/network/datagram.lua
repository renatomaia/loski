local time = require "time"
local network = require "network"
local tests = require "test.network.utils"

local packsize = 64
local data = string.rep("X", packsize)
local final = string.rep("X", packsize-1).."0"
local remotecode = [[
	local time = require "time"
	local network = require "network"

	local socket = assert(network.socket("datagram"))
	assert(socket:setoption("reuseaddr", true) == true)
	assert(socket:bind("*", ]]..tests.LocalPort..[[) == true)
	local packsize = ]]..packsize..[[
	local all = {}
	local i = 0
	repeat
		i = i+1
		all[i], host, port = assert(socket:receive(packsize, true))
		assert(host == "]]..tests.LocalHost..[[")
		assert(port == ]]..tests.FreePort..[[)
	until string.find(all[i], "0", nil, "noregex")
	for _, data in ipairs(all) do
		time.sleep(.1)
		assert(socket:send(data, 1, -1, "]]..tests.LocalHost..[[", ]]..tests.FreePort..[[) == #data)
	end
	assert(socket:close())
]]

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("datagram")

	assert(socket:bind(tests.LocalHost, tests.FreePort) == true)
	assert(socket:connect(tests.LocalHost, tests.LocalPort) == true)
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

	assert(socket:bind(tests.LocalHost, tests.FreePort) == true)
	for i = 1, 3 do
		assert(socket:send(data, 1, -1, tests.LocalHost, tests.LocalPort) == packsize)
	end
	assert(socket:send(final, 1, -1, tests.LocalHost, tests.LocalPort) == packsize)
	local remaining = 3*packsize
	while remaining > 0 do
		local received, host, port = socket:receive(remaining, true)
		assert(received == data)
		assert(host == tests.LocalHost)
		assert(port == tests.LocalPort)
		remaining = remaining - #data
	end
	local received, host, port = socket:receive(packsize, true)
	assert(received == final)
	assert(host == tests.LocalHost)
	assert(port == tests.LocalPort)

	tests.testclose(socket)
end

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("datagram")
	assert(socket:setoption("blocking", false) == true)

	assert(socket:bind(tests.LocalHost, tests.FreePort) == true)
	assert(socket:connect(tests.LocalHost, tests.LocalPort) == true)

	--[[
	local res, errmsg
	repeat
		res, errmsg = socket:send(data)
		data = data..data
	until res == nil
	assert(errmsg == "unfulfilled")
	--[=[--]]
	for i = 1, 3 do
		assert(socket:send(data) == packsize)
	end
	--]=]

	tests.testerrmsg("unfulfilled", socket:receive(packsize))
	assert(socket:send(final) == packsize)
	local remaining = 3*packsize
	while remaining > 0 do
		local received, host, port = tests.tcall(true, socket.receive, socket, remaining, true)
		assert(received == data)
		assert(host == tests.LocalHost)
		assert(port == tests.LocalPort)
		remaining = remaining - #data
	end
	assert(tests.tcall(true, socket.receive, socket, packsize) == final)

	tests.testclose(socket)
end
