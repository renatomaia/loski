local WIN = require("os").getenv("PATHEXT")~=nil

local network = require "network"

for _, kind in ipairs{"datagram", "connection", "listen"} do
	-- socket = network.socket(type)
	local socket = network.socket(kind)
	assert(type(socket) == "userdata")
	assert(string.match(tostring(socket), "^socket %(.-%)$"))

	-- succ [, errmsg] = socket:bind(host, port)
	assert(socket:bind("localhost", 54321) == true)
	local res, errmsg = socket:bind("localhost", 54320)
	assert(res == nil)
	assert(errmsg == "invalid operation")

	-- host, port = socket:getaddress([site])
	local host, port = socket:getaddress("local")
	assert(host == "127.0.0.1")
	assert(port == 54321)
	local res, errmsg = socket:getaddress("remote")
	assert(res == nil)
	assert(errmsg == "disconnected")
	local res, errmsg = socket:getaddress()
	assert(res == nil)
	assert(errmsg == "disconnected")

	-- value [, errmsg] = socket:getoption(name)
	assert(socket:getoption("blocking") == true)
	assert(socket:getoption("reuseaddr") == false)
	assert(socket:getoption("dontroute") == false)
	if WIN then
		if kind == "datagram" then
			for _, option in ipairs{"keepalive", "linger", "nodelay"} do
				local res, errmsg = socket:getoption(option)
				assert(res == nil)
				assert(errmsg == "unsupported")
			end
			assert(socket:getoption("broadcast") == false)
		else
			assert(socket:getoption("keepalive") == false)
			assert(socket:getoption("linger") == 0)
			assert(socket:getoption("nodelay") == false)
			local res, errmsg = socket:getoption("broadcast")
			assert(res == nil)
			assert(errmsg == "unsupported")
		end
	else
		assert(socket:getoption("keepalive") == false)
		assert(socket:getoption("linger") == 0)
		if kind == "datagram" then
			local res, errmsg = socket:getoption("nodelay")
			assert(res == nil)
			assert(errmsg == "invalid operation")
		else
			assert(socket:getoption("nodelay") == false)
		end
		assert(socket:getoption("broadcast") == false)
	end

	-- succ [, errmsg] = socket:setoption(name, value)
	assert(socket:setoption("blocking", false) == true)
	assert(socket:setoption("reuseaddr", true) == true)
	assert(socket:setoption("dontroute", true) == true)
	if WIN then
		if kind == "datagram" then
			assert(socket:setoption("broadcast", true) == true)
			for option, value in pairs{keepalive=true, linger=10, nodelay=true} do
				local res, errmsg = socket:setoption(option, value)
				assert(res == nil)
				assert(errmsg == "unsupported")
			end
			assert(socket:setoption("broadcast", true) == true)
		else
			assert(socket:setoption("keepalive", true) == true)
			assert(socket:setoption("linger", 10) == true)
			assert(socket:setoption("nodelay", true) == true)
			local res, errmsg = socket:getoption("broadcast")
			assert(res == nil)
			assert(errmsg == "unsupported")
		end
	else
		assert(socket:setoption("keepalive", true) == true)
		assert(socket:setoption("linger", 10) == true)
		if kind == "datagram" then
			local res, errmsg = socket:setoption("nodelay", true)
			assert(res == nil)
			assert(errmsg == "invalid operation")
		else
			assert(socket:setoption("nodelay", true) == true)
		end
		assert(socket:setoption("broadcast", true) == true)
	end
	assert(socket:getoption("blocking") == false)
	assert(socket:getoption("reuseaddr") == true)
	assert(socket:getoption("dontroute") == true)
	if WIN then
		if kind == "datagram" then
			for _, option in ipairs{"keepalive", "linger", "nodelay"} do
				local res, errmsg = socket:getoption(option)
				assert(res == nil)
				assert(errmsg == "unsupported")
			end
			assert(socket:getoption("broadcast") == true)
		else
			assert(socket:getoption("keepalive") == true)
			assert(socket:getoption("linger") == 10)
			assert(socket:getoption("nodelay") == true)
			local res, errmsg = socket:getoption("broadcast")
			assert(res == nil)
			assert(errmsg == "unsupported")
		end
	else
		assert(socket:getoption("keepalive") == true)
		assert(socket:getoption("linger") == 10)
		if kind == "datagram" then
			local res, errmsg = socket:getoption("nodelay")
			assert(res == nil)
			assert(errmsg == "invalid operation")
		else
			assert(socket:getoption("nodelay") == true)
		end
		assert(socket:getoption("broadcast") == true)
	end

	-- succ [, errmsg] = socket:close()
	assert(socket:close() == true)
	for _, op in ipairs{"close", "bind", "getaddress", "getoption", "setoption"} do
		local ok, errmsg = pcall(socket[op], socket)
		assert(not ok)
		assert(errmsg == "attempt to use a closed socket")
	end
end
