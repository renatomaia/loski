local WIN = require("os").getenv("PATHEXT")~=nil

local network = require "network"

for _, kind in ipairs{"datagram", "connection"} do
	-- socket = network.socket(type)
	local socket = network.socket(kind)
	assert(type(socket) == "userdata")
	assert(string.match(tostring(socket), "^socket %(.-%)$"))

	-- succ [, errmsg] = socket:connect(host, port)
	if WIN then
		local res, errmsg = socket:connect("*", 80)
		assert(res == nil)
		assert(errmsg == "address unavailable")
	else
		if kind == "datagram" then
			assert(socket:connect("*", 80) == true)
		else
			local res, errmsg = socket:connect("*", 80)
			assert(res == nil)
			assert(errmsg == "connection refused")
		end
	end

	-- succ [, errmsg] = socket:close()
	assert(socket:close() == true)
	for _, op in ipairs{
		"close",
		"bind",
		"getaddress",
		"getoption",
		"setoption",
		"connect",
		"send",
		"receive",
	} do
		local ok, errmsg = pcall(socket[op], socket)
		assert(not ok)
		assert(errmsg == "attempt to use a closed socket")
	end
end

for _, kind in ipairs{"datagram", "connection"} do
	-- socket = network.socket(type)
	local socket = network.socket(kind)
	assert(type(socket) == "userdata")
	assert(string.match(tostring(socket), "^socket %(.-%)$"))

	assert(socket:connect("www.google.com", 80) == true)
	if kind == "datagram" then
		assert(socket:connect("www.google.com.br", 80) == true)
	else
		local res, errmsg = socket:connect("www.google.com.br", 80)
		assert(res == nil)
		assert(errmsg == "connected")
	end

	-- succ [, errmsg] = socket:close()
	assert(socket:close() == true)
	for _, op in ipairs{
		"close",
		"bind",
		"getaddress",
		"getoption",
		"setoption",
		"connect",
		"send",
		"receive",
	} do
		local ok, errmsg = pcall(socket[op], socket)
		assert(not ok)
		assert(errmsg == "attempt to use a closed socket")
	end
end
