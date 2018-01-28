local event = require "event"
local network = require "network"
local time = require "time"
local testutils = require "test.utils"

do
	local watcher = assert(event.watcher())
	local adr = assert(network.resolve("localhost", 0)())

	local lst = assert(network.socket("listen"))
	assert(lst:setoption("blocking", false))
	assert(lst:bind(adr))
	assert(lst:getaddress("this", adr))
	assert(lst:listen())
	assert(watcher:set(lst, "r"))
	assert(next(watcher:wait(0)) == nil)
	local start = time.now()
	assert(next(watcher:wait(.5)) == nil)
	assert(time.now() - start > .5)

	local cnt = assert(network.socket("connection"))
	assert(cnt:setoption("blocking", false))
	testutils.testerrmsg("unfulfilled", cnt.connect(cnt, adr))
	assert(watcher:set(cnt, "w"))
	local events = assert(watcher:wait(0))
	assert(events[cnt] == "w")
	assert(events[lst] == "r")
	assert(cnt:connect(adr))
	assert(watcher:set(cnt, "r"))
	local srv = assert(lst:accept())
	assert(srv:setoption("blocking", false))
	assert(watcher:set(srv, "r"))
	assert(next(watcher:wait(0)) == nil)

	assert(cnt:send("Hello!"))
	assert(srv:send("Hi!"))
	local events = assert(watcher:wait(0))
	assert(events[lst] == nil)
	assert(events[cnt] == "r")
	assert(events[srv] == "r")
	assert(cnt:receive(3) == "Hi!")
	assert(srv:receive(6) == "Hello!")
	assert(next(watcher:wait(0)) == nil)

	assert(cnt:send("Hi there!"))
	assert(srv:send("Hello back!"))
	local events = assert(watcher:wait(0))
	assert(events[lst] == nil)
	assert(events[cnt] == "r")
	assert(events[srv] == "r")
	assert(cnt:receive(11) == "Hello back!")
	assert(srv:receive(9) == "Hi there!")
	assert(next(watcher:wait(0)) == nil)

	assert(watcher:set(cnt, "rw"))
	assert(watcher:set(srv, "rw"))
	assert(cnt:send("OK, bye."))
	assert(srv:send("Good bye."))

	local events = assert(watcher:wait(0))
	assert(events[lst] == nil)
	assert(events[cnt] == "rw")
	assert(events[srv] == "rw")
	assert(cnt:receive(9) == "Good bye.")
	assert(srv:receive(8) == "OK, bye.")
	local events = assert(watcher:wait(0))
	assert(events[lst] == nil)
	assert(events[cnt] == "w")
	assert(events[srv] == "w")

	--TODO: avoid closing sources that are inserted in a watcher
	assert(cnt:close())
	assert(srv:close())
	testutils.testerror("invalid operation", watcher.wait, watcher)
	--assert(watcher:set(cnt, nil))
	--assert(watcher:set(srv, nil))

	--assert(next(watcher:wait(0)) == nil)
	--assert(lst:close())
	--testutils.testerror("invalid operation", watcher.wait, watcher)
end

do
	local process = require "process"

	local port = 54322
	local szlen = 4
	local msgfmt = "%"..szlen.."d%s"
	local luabin = "lua"
	do
		local i = -1
		while arg[i] ~= nil do
			luabin = arg[i]
			i = i-1
		end
	end

	local server = assert(process.create(luabin, "-e", [[
	local event = require "event"
	local network = require "network"

	local msgfmt = ]]..string.format("%q", msgfmt)..[[

	local addr = assert(network.resolve("localhost", ]]..port..[[)())
	local port = assert(network.socket("listen"))
	assert(port:bind(addr))
	assert(port:listen())

	local watcher = assert(event.watcher())
	assert(watcher:set(port, "r"))
	local count = 0
	repeat
		local events = assert(watcher:wait())
		for socket in pairs(events) do
			if socket == port then
				local conn = assert(port:accept())
				assert(watcher:set(conn, "r"))
				count = count+1
			else
				local conn = socket
				local size = assert(tonumber(assert(conn:receive(]]..szlen..[[))))
				if size > 0 then
					local data = assert(conn:receive(size))
					local result = assert(load(data))()
					local msg = msgfmt:format(#result, result)
					assert(assert(conn:send(msg)) == #msg)
				else
					assert(watcher:set(conn, nil))
					assert(conn:close())
					count = count-1
				end
			end
		end
	until count == 0
	assert(watcher:set(port, nil))
	assert(watcher:close())
	assert(port:close())
	]]))

	time.sleep(1)

	local conns = {}
	local addr = assert(network.resolve("localhost", port)())
	for i = 1, 3 do
		conns[i] = assert(network.socket("connection"))
		assert(conns[i]:connect(addr))
	end

	local code = "return _VERSION"
	for i = 3, 1, -1 do
		time.sleep(i)
		local conn = conns[i]
		local msg = msgfmt:format(#code, code)
		assert(assert(conn:send(msg)) == #msg)
		local size = assert(conn:receive(szlen))
		local data = assert(conn:receive(size))
		assert(data == _VERSION)
	end

	for i = 1, 3 do
		time.sleep(i)
		local conn = conns[i]
		assert(conn:send(msgfmt:format(0, "")))
		assert(conn:close())
	end
end
