local time = require "time"
local network = require "network"
local process = require "process"

local port = ...
local szlen = 4
local msgfmt = "%"..szlen.."d%s"

local server = assert(process.create("lua", "-e", [[
local event = require "event"
local network = require "network"

local msgfmt = ]]..string.format("%q", msgfmt)..[[

local port = assert(network.socket("listen"))
assert(port:bind("*", ]]..port..[[))
assert(port:listen())

local watcher = assert(event.watcher())
watcher:add(port, "read")
local count = 0
repeat
	local events = assert(watcher:wait())
	for socket in pairs(events) do
		if socket == port then
			local conn = assert(port:accept())
			watcher:add(conn, "read")
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
				watcher:remove(conn, "read")
				assert(conn:close())
				count = count-1
			end
		end
	end
until count == 0
watcher:remove(port, "read")
assert(port:close())
]]))

time.sleep(1)

local conns = {}

for i = 1, 3 do
	conns[i] = assert(network.socket("connection"))
	assert(conns[i]:connect("localhost", port))
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
