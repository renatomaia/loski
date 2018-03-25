local _G = require "_G"
local assert = _G.assert
local error = _G.error
local getmetatable = _G.getmetatable
local ipairs = _G.ipairs
local pcall = _G.pcall
local tonumber = _G.tonumber
local tostring = _G.tostring
local type = _G.type

local math = require "math"
local max = math.max
local min = math.min

local string = require "string"
local strfind = string.find
local strmatch = string.match
local strsub = string.sub

local debug = require "debug"
local setmetatable = debug.setmetatable

local vararg = require "vararg"
local varange = vararg.range

local memory = require "memory"
local memalloc = memory.create
local memfill = memory.fill
local memfind = memory.find
local memget = memory.get
local memrealloc = memory.resize
local mem2str = memory.tostring

local time = require "time"
local now = time.now
local sleep = time.sleep

local network = require "network"
local newaddr = network.address
local newsocket = network.socket
local resolveaddr = network.resolve

local event = require "event"
local newpoll = event.watcher

local defaultsize = 8192

local function checkclass(self, expected)
	if self.class ~= expected then
		local actual = self.class or type(self)
		error("bad argument #1 ("..expected.." expected, but got "..actual..")")
	end
end

local toaddr do
	local addrset = getmetatable(newaddr("ipv4")).__newindex

	function toaddr(addr, host, port)
		if not pcall(addrset, addr, "literal", host) then
			local next, errmsg = resolveaddr(host, port)
			if next == nil then return nil, errmsg end
			next(addr)
		else
			addr.port = port
		end
		return addr
	end
end

local function wrapsock(class, metatable, domain, result, errmsg)
	if result == nil then return nil, errmsg end
	return setmetatable({
		socket = result,
		address = newaddr(domain),
		buffer = memalloc(defaultsize),
		class = class,
	}, metatable)
end

local event2pollfield = {
	r = "readready",
	w = "writeready",
}

local function freesockpoll(self, event)
	local field = event2pollfield[event]
	local poll = self[field]
	if poll ~= nil then
		poll:close()
		self[field] = nil
	end
end

local function dotimeout(self, event, result, errmsg, start)
	if result == nil and errmsg == "unfulfilled" then
		local timeout = self.syscalltimeout
		local total = self.totaltimeout
		if total ~= nil then
			if start ~= nil then
				total = total - (now() - start)
			elseif total > 0 then
				start = now()
			end
			timeout = timeout == nil and total or min(timeout, total)
		end
		local field = event2pollfield[event]
		local ready = self[field]:wait(timeout)
		if ready == nil or ready[self.socket] == nil then
			errmsg = "timeout"
		else
			result, errmsg = 0
		end
	end
	return result, errmsg, start
end

local function connectsock(self, host, port, class)
	local socket = self.socket
	if socket == nil then return nil, "closed" end
	local addr, errmsg = toaddr(self.address, host, port)
	if addr == nil then return nil, errmsg end
	local result, start
	repeat
		result, errmsg = socket:connect(addr)
		result, errmsg, start = dotimeout(self, "w", result, errmsg, start)
	until result ~= 0
	if result == nil and errmsg ~= "in use" then return nil, errmsg end
	self.class = class
	return true
end

local tcp = {
	class = "tcp{master}",
	sendmaxsz = defaultsize,
	first = 1,
	last = 0,
}
tcp.__index = tcp

do
	local function rawtostring(value)
		local mt = getmetatable(value)
		local tostr = mt.__tostring
		if tostr ~= nil then
			mt.__tostring = nil
		end
		local str = tostring(value)
		if tostr ~= nil then
			mt.__tostring = tostr
		end
		return strmatch(str, "%w+: (0x%x+)$")
	end

	function tcp:__tostring()
		return self.class..": "..rawtostring(self)
	end
end

function tcp:accept()
	checkclass(self, "tcp{server}")
	local socket = self.socket
	if socket == nil then return nil, "closed" end
	local result, errmsg, start
	repeat
		result, errmsg = socket:accept()
		result, errmsg, start = dotimeout(self, "r", result, errmsg, start)
	until result ~= 0
	return wrapsock("tcp{client}", tcp, self.address.type, result, errmsg)
end

function tcp:bind(host, port)
	return self.socket:bind(toaddr(self.address, host, port))
end

function tcp:close()
	local socket = self.socket
	if socket == nil then return nil, "closed" end
	self.syscalltimeout = nil
	self.totaltimeout = nil
	freesockpoll(self, "r")
	freesockpoll(self, "w")
	self.socket = nil
	return socket:close()
end

function tcp:connect(host, port)
	checkclass(self, "tcp{master}")
	return connectsock(self, host, port, "tcp{client}")
end

function tcp:dirty()
	return self.first <= self.last
end

function tcp:getfd()
	return tonumber(strmatch(tostring(self.socket), "socket %((%d+)%)") or -1)
end

do
	local function getaddr(self, side)
		local addr = self.address
		local res, errmsg = self.socket:getaddress(side, addr)
		if res == nil then return nil, errmsg end
		return addr.literal, addr.port
	end

	function tcp:getpeername()
		return getaddr(self, "peer")
	end

	function tcp:getsockname()
		return getaddr(self, "this")
	end
end

function tcp:getstats()
	checkclass(self, "tcp{client}")
	error("not supported yet")
end

do
	local lstmt do
		local lstsck = assert(newsocket("listen"))
		lstmt = getmetatable(lstsck)
		lstsck:close()
	end

	function tcp:listen(backlog)
		checkclass(self, "tcp{master}")
		local socket = self.socket
		if socket == nil then return nil, "closed" end
		local oldmt = getmetatable(socket)
		setmetatable(socket, lstmt)
		local result, errmsg = socket:listen(backlog)
		if result == nil then
			setmetatable(socket, oldmt)
			return nil, errmsg
		end
		self.class = "tcp{server}"
		return true
	end
end

do
	local pat2term = { ["*l"] = "\n" }

	local function initbuf(self, prefix, required)
		local buffer = self.buffer
		local first = self.first
		local last = self.last
		local datasz = 1+last-first
		local pfxsz = #prefix
		local pfxidx = first-pfxsz
		local bufsz = #buffer
		local newbuf
		local reqsz = required == nil and 1 or max(0, required-datasz)
		if pfxsz+datasz+reqsz <= bufsz then
			newbuf = buffer
		end
		if newbuf == buffer and pfxidx > 0 and first+datasz+reqsz-1 <= bufsz then
			if pfxsz > 0 then memfill(buffer, prefix, pfxidx, first-1) end
		else
			local dataidx = first
			pfxidx, first, last = 1, pfxsz+1, pfxsz+datasz
			if newbuf == nil then
				bufsz = last+max(reqsz, bufsz)
				newbuf = memalloc()
				memrealloc(newbuf, bufsz)
			end
			if datasz > 0 then memfill(newbuf, buffer, first, last, dataidx) end
			if pfxsz > 0 then memfill(newbuf, prefix, 1, pfxsz) end
		end
		return newbuf, pfxidx, first, last
	end

	local function incbuf(self, buffer)
		local defbuf = self.buffer
		local bufsz = #buffer
		local newsz = bufsz+#defbuf
		if buffer == defbuf then
			buffer = memalloc()
			memrealloc(buffer, newsz)
			memfill(buffer, defbuf, 1, bufsz)
		else
			memrealloc(buffer, newsz)
		end
		return buffer
	end

	function tcp:receive(pattern, prefix)
		checkclass(self, "tcp{client}")
		if self.closed then return nil, "closed" end
		if pattern == nil then pattern = "*l" end
		if prefix == nil then prefix = "" end
		local result, errmsg, partial = 0 -- to signal success
		local pattype = type(pattern)
		local required = (pattype == "number") and pattern or nil
		local buffer, pfxidx, first, last = initbuf(self, prefix, required)

		local socket = self.socket
		if socket == nil then return nil, "closed" end
		local start, reqidx
		if pattype == "number" then
			reqidx = pfxidx+pattern-1
			while reqidx > last do
				result, errmsg = socket:receive(buffer, last+1, reqidx)
				result, errmsg, start = dotimeout(self, "r", result, errmsg, start)
				if result == nil then
					reqidx = last
					break
				end
				last = last+result
			end
			partial = mem2str(buffer, pfxidx, reqidx)
		else
			local term = pat2term[pattern]
			assert(term ~= nil or pattern == "*a", "invalid pattern")
			while true do
				reqidx = term and memfind(buffer, term, first, last)
				if reqidx ~= nil then
					break
				else
					first = last+1
					if first > #buffer then buffer = incbuf(self, buffer) end
					result, errmsg = socket:receive(buffer, first)
					result, errmsg, start = dotimeout(self, "r", result, errmsg, start)
					if result == nil then
						reqidx = last
						break
					end
					last = last+result
				end
			end
			local endidx = reqidx
			if result ~= nil then
				if term == "\n" and memget(buffer, reqidx-1) == 13 then
					endidx = reqidx-2
				else
					endidx = reqidx-#term
				end
			end
			partial = mem2str(buffer, pfxidx, endidx)
			if pattern == "*a" and errmsg == "closed" and #partial > 0 then
				result = 0 -- to signal success
			end
		end
		first = reqidx+1

		if first > last then
			self.first, self.last = nil, nil
		elseif buffer == self.buffer then
			self.first, self.last = first, last
		else
			local datasz = 1+last-first
			memfill(self.buffer, buffer, 1, datasz, first)
			self.first, self.last = 1, datasz
		end

		if result == nil then
			if errmsg == "aborted" then errmsg = "closed" end
			return nil, errmsg, partial
		end
		return partial
	end
end

function tcp:send(data, first, last)
	checkclass(self, "tcp{client}")
	if first == nil then first = 1 end
	if last == nil then last = #data end
	local socket = self.socket
	if socket == nil then return nil, "closed" end
	local sendmaxsz = self.sendmaxsz
	local start
	local i = first
	while i <= last do
		local sent, errmsg = socket:send(data, i, min(last, i+sendmaxsz))
		sent, errmsg, start = dotimeout(self, "w", sent, errmsg, start)
		if sent == nil then
			if errmsg == "aborted" then errmsg = "closed" end
			return nil, errmsg, i-1
		end
		i = i+sent
	end
	return i-1, nil, nil
end

function tcp:setfd(fd)
	error("not supported yet")
end

do
	local name2opt = {
		keepalive = "keepalive",
		reuseaddr = "reuseaddr",
		["tcp-nodelay"] = "nodelay",
		linger = "linger",
	}

	function tcp:setoption(name, value)
		local option = assert(name2opt[name], "unsupported option")
		if name == "linger" then
			value = value.on and value.timeout or nil
		end
		return self.socket:setoption(option, value)
	end
end

tcp.setpeername = tcp.connect
tcp.setsockname = tcp.bind

function tcp:setstats()
	checkclass(self, "tcp{client}")
	error("not supported yet")
end

do
	local function newsockpoll(self, event)
		local field = event2pollfield[event]
		local poll = self[field]
		if poll == nil then
			poll = assert(newpoll())
			assert(poll:set(self.socket, event))
			self[field] = poll
		end
		return poll
	end

	local mode2field = {
		b = "syscalltimeout",
		t = "totaltimeout",
		r = "totaltimeout",
	}

	function tcp:settimeout(value, mode)
		local field = assert(mode2field[strsub(mode or "b", 1, 1)], "invalid mode")
		if value ~= nil then
			assert(type(value) == "number", "number expected")
			if value < 0 then value = nil end
		end
		self[field] = value
		local hastimeout = (self.syscalltimeout ~= nil or self.totaltimeout ~= nil)
		if hastimeout then
			newsockpoll(self, "r")
			newsockpoll(self, "w")
		else
			freesockpoll(self, "r")
			freesockpoll(self, "w")
		end
		assert(self.socket:setoption("blocking", not hastimeout))
	end
end

function tcp:shutdown(mode)
	checkclass(self, "tcp{client}")
	return self.socket:shutdown(mode)
end

local udp = {
	class = "udp{unconnected}",
	close = tcp.close,
	getfd = tcp.getfd,
	getpeername = tcp.getpeername,
	getsockname = tcp.getsockname,
	setfd = tcp.setfd,
	setoption = tcp.setoption,
	setsockname = tcp.bind,
	settimeout = tcp.settimeout,
}
udp.__index = udp

function udp:dirty()
	return false
end

do
	local function recvdgram(self, size, ...)
		local socket = self.socket
		if socket == nil then return nil, "closed" end
		local buffer = self.buffer
		local result, errmsg, start
		repeat
			result, errmsg = socket:receive(buffer, 1, size, ...)
			result, errmsg, start = dotimeout(self, "r", result, errmsg, start)
			if result == nil then return nil, errmsg end
		until result == size or result > 0
		return mem2str(buffer, 1, result)
	end

	function udp:receive(size)
		checkclass(self, "udp{connected}")
		return recvdgram(self, size)
	end

	function udp:receivefrom(size)
		checkclass(self, "udp{unconnected}")
		local addr = self.address
		local result, errmsg = recvdgram(self, size, nil, addr)
		if result == nil then return nil, errmsg end
		return result, addr.literal, addr.port
	end
end

function udp:send(data)
	checkclass(self, "udp{connected}")
	return self.socket:send(data)
end

function udp:sendto(data, host, port)
	checkclass(self, "udp{unconnected}")
	return self.socket:send(data, 1, -1, toaddr(self.address, host, port))
end

function udp:setpeername(host, port)
	return connectsock(self, host, port, host == "*" and "udp{unconnected}"
	                                                  or "udp{connected}")
end

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

local socket = {
	_VERSION = "LuaSocket 2.0.2",
	dns = dns,
	gettime = now,
	sleep = time.sleep,
}

function socket.newtry(func)
	return function (ok, ...)
		if not ok then
			pcall(func)
			error{(...)}
		end
		return ...
	end
end

do
	local function cont(ok, ...)
		if not ok then
			local err = ...
			if type(err) == "table" then
				return nil, err[1]
			end
			error(err)
		end
		return ...
	end
	function socket.protect(func)
		return function (...)
			return cont(pcall(func, ...))
		end
	end
end

do
	function socket.tcp()
		return wrapsock(nil, tcp, "ipv4", newsocket("stream", "ipv4"))
	end

	function socket.udp()
		return wrapsock(nil, udp, "ipv4", newsocket("datagram", "ipv4"))
	end

	local sockcls = { [tcp]=true, [udp]=true }

	local function addresult(list, sock)
		list[#list+1] = sock
		list[sock] = true
	end

	local function addsocks(watcher, list, result, event)
		local added = false
		if list ~= nil then
			for _, value in ipairs(list) do
				local sock = value.socket
				if sockcls[getmetatable(sock)] ~= nil then
					if sock:dirty() then
						addresult(result, value)
					elseif watcher:set(sock, event) then
						added = true
					end
				end
			end
		end
		return added
	end

	local function filtersocks(map, list, result, event)
		for index, value in ipairs(list) do
			local ready = map[value.socket]
			if ready ~= nil and strfind(ready, event, 1, true) ~= nil then
				addresult(result, value)
			end
		end
	end

	function socket.select(recvt, sendt, timeout)
		local recvok, sendok, errmsg = {}, {}
		local poll = newpoll()
		if addsocks(poll, recvt, recvok, "r") or
		   addsocks(poll, sendt, sendok, "w") then
			local map
			map, errmsg = poll:wait(timeout)
			if map ~= nil then
				filtersocks(map, recvt, recvok, "r")
				filtersocks(map, sendt, sendok, "w")
			elseif errmsg == "unfulfilled" then
				errmsg = "timeout"
			end
		else
			sleep(timeout)
			errmsg = "timeout"
		end
		poll:close()
		return recvok, sendok, errmsg
	end
end

function socket.skip(c, ...)
	varange(1, -c, ...)
end

function socket.__unload()
	-- empty
end

if LUASOCKET_DEBUG then
	local function wrap(func)
		return function (...)
			local start = now()
			local result, errmsg, partial = func(...)
			return result, errmsg, partial, now() - start
		end
	end
	tcp.receive = wrap(tcp.receive)
	tcp.send = wrap(tcp.send)
	socket._DEBUG = true
end

return socket
