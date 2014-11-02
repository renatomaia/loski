local time = require "time"
local network = require "network"
local tests = require "test.network.utils"

local packsize = 64
local data = string.rep("X", packsize)
local final = string.rep("X", packsize-1).."0"
local remotecode = [[
	local time = require "time"
	local network = require "network"

	local port = assert(network.socket("listen"))
	assert(port:setoption("reuseaddr", true) == true)
	assert(port:bind("*", ]]..tests.LocalPort..[[) == true)
	assert(port:listen() == true)
	local conn = assert(port:accept())
	local packsize = ]]..packsize..[[
	local all = {}
	local i = 0
	repeat
		i = i+1
		all[i] = assert(conn:receive(packsize))
	until string.find(all[i], "0", nil, "noregex")
	for _, data in ipairs(all) do
		time.sleep(.1)
		assert(conn:send(data) == #data)
	end
	assert(conn:close())
	assert(port:close())
]]

do
	tests.newprocess(remotecode)
	time.sleep(1)

	local socket = tests.testcreatesocket("connection")

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

	local socket = tests.testcreatesocket("connection")
	assert(socket:setoption("blocking", false) == true)

	tests.testerrmsg("connected",
		tests.tcall(true, socket.connect, socket, tests.LocalHost, tests.LocalPort))

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
		assert(tests.tcall(true, socket.receive, socket, remaining) == data)
		remaining = remaining - #data
	end
	assert(tests.tcall(true, socket.receive, socket, packsize) == final)

	tests.testclose(socket)
end
