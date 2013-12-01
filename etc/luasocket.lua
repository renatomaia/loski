local network = require "network"

local dns = {}

function dns.gethostname()
	error("not supported yet")
end

function dns.tohostname()
	error("not supported yet")
end

function dns.toip()
	error("not supported yet")
end

local tcp = { class = "tcp{master}" }

function tcp:__tostring()
	return self.class..": "..rawtostring(self)
end

function tcp:accept()
	return self.socket:accept()
end

function tcp:bind(host, port)
	return self.socket:bind(host, port)
end

function tcp:close()
	return self.socket:close()
end

function tcp:connect(host, port)
	return self.socket:connect(host, port)
end

function tcp:dirty()
	return self.partial ~= nil
end

function tcp:getfd()
	error("not supported yet")
end

function tcp:getpeername()
	return self.socket:getaddress("remote")
end

function tcp:getsockname()
	return self.socket:getaddress("local")
end

function tcp:getstats()
	error("not supported yet")
end

function tcp:listen(backlog)
	return self.socket:listen(backlog)
end

function tcp:receive(pattern, prefix)
	local 
end

function tcp:send()
	error("not supported yet")
end

function tcp:setfd()
	error("not supported yet")
end

function tcp:setoption()
	error("not supported yet")
end

function tcp:setpeername()
	error("not supported yet")
end

function tcp:setsockname()
	error("not supported yet")
end

function tcp:setstats()
	error("not supported yet")
end

function tcp:settimeout()
	error("not supported yet")
end

function tcp:shutdown()
	error("not supported yet")
end

local udp = {}

function udp:close()
	error("not supported yet")
end

function udp:dirty()
	error("not supported yet")
end

function udp:getfd()
	error("not supported yet")
end

function udp:getpeername()
	error("not supported yet")
end

function udp:getsockname()
	error("not supported yet")
end

function udp:receive()
	error("not supported yet")
end

function udp:receivefrom()
	error("not supported yet")
end

function udp:send()
	error("not supported yet")
end

function udp:sendto()
	error("not supported yet")
end

function udp:setfd()
	error("not supported yet")
end

function udp:setoption()
	error("not supported yet")
end

function udp:setpeername()
	error("not supported yet")
end

function udp:setsockname()
	error("not supported yet")
end

function udp:settimeout()
	error("not supported yet")
end

local socket = {
	_VERSION = "LuaSocket 2.0.2",
	dns = dns,
}

function socket.newtry()
	error("not supported yet")
end

function socket.protect()
	error("not supported yet")
end

function socket.gettime()
	error("not supported yet")
end

function socket.sleep()
	error("not supported yet")
end

function socket.tcp()
	error("not supported yet")
end

function socket.udp()
	error("not supported yet")
end

function socket.select()
	error("not supported yet")
end

function socket.skip()
	error("not supported yet")
end

function socket.__unload()
	error("not supported yet")
end

return socket
