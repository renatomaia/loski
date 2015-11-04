local network = require "network"

local tests = {
	IsWindows = os.getenv("PATHEXT")~=nil,
	FreePort = 43210,
	UsedPort = 54321,
	DeniedPort = 1,
	LocalHost = "127.0.0.1",
	LocalPort = 43212,
	RemoteTCP = {host="www.google.com",port=80},
	OtherTCP = {host="www.google.com.br",port=80},
	RemoteUDP = {host="www.google.com",port=80},
}

do
	local socket = assert(network.socket("listen"))
	local res, errmsg = socket:bind("*", tests.UsedPort)
	assert(res == true or errmsg == "address used")
	tests.BindedSocket = socket -- avoid garbage collection
end

do
	local process = require "process"

	local luabin = "lua"
	do
		local i = -1
		while arg[i] ~= nil do
			luabin = arg[i]
			i = i-1
		end
	end

	function tests.newprocess(code)
		return assert(process.create(luabin, "-e", code))
	end
end

do
	local time = require "time"

	function tests.tcall(unfulfilled, func, ...)
		for i = 1, 10 do
			local results = table.pack(func(...))
			local res, errmsg = results[1], results[2]
			if res ~= nil or errmsg ~= "unfulfilled" then
				if unfulfilled then assert(res == nil or i > 1) end
				return table.unpack(results, 1, results.n)
			end
			time.sleep(.1)
		end
		error("non blocked call took more than 1 second to complete!")
	end
end

do
	function tests.testerror(expected, func, ...)
		local ok, errmsg = pcall(func, ...)
		assert(not ok)
		assert(errmsg == expected, errmsg)
	end
end

do
	function tests.testerrmsg(expected, res, errmsg)
		assert(res == nil)
		assert(errmsg == expected)
	end
end

do
	function tests.testcreatesocket(kind)
		local socket = network.socket(kind)
		assert(type(socket) == "userdata")
		assert(string.match(tostring(socket), "^socket %(.-%)$"))
		return socket
	end
end

do
	local options = {
		blocking = true,
		reuseaddr = false,
		dontroute = false,
		nodelay = false,
		keepalive = false,
		linger = 0,
		broadcast = false,
	}
	local changed = {
		[true] = false,
		[false] = true,
		[0] = 10,
	}
	local disallowed = {
		listen = {
			nodelay = true,
			keepalive = true,
			linger = true,
			broadcast = true,
		},
		connection = {
			broadcast = true,
		},
		datagram = {
			nodelay = true,
			keepalive = true,
			linger = true,
		},
	}
	local unsupported = {
		listen = {},
		connection = {
			reuseaddr = not tests.IsWindows,
			dontroute = not tests.IsWindows,
			nodelay = not tests.IsWindows,
			keepalive = not tests.IsWindows,
			linger = not tests.IsWindows,
		},
		datagram = {},
	}
--print(kind..":getoption("..name..") == ",socket:getoption(name))
--print(refused, kind..":setoption("..name..", "..tostring(changed[default])..") == ",socket:setoption(name, changed[default]))
	function tests.testoptions(socket, kind, refused)
		local disallowed = disallowed[kind]
		for name, default in pairs(options) do
			if disallowed[name] then
				local errmsg = "bad argument #2 to '?' (invalid option '"..name.."')"
				tests.testerror(errmsg, socket.getoption, socket, name)
				local errmsg = "bad argument #2 to '?' (invalid option '"..name.."')"
				tests.testerror(errmsg, socket.setoption, socket, name, changed[default])
			else
				assert(socket:getoption(name) == default)
				if refused and unsupported[kind][name] then
					tests.testerrmsg("unsupported", socket:setoption(name, changed[default]))
				else
					assert(socket:setoption(name, changed[default]) == true)
					assert(socket:getoption(name) == changed[default])
					assert(socket:setoption(name, default) == true)
				end
			end
		end
	end
end

do
	local ops = {"close", "bind", "getaddress", "getoption", "setoption"}
	function tests.testclose(socket)
		assert(socket:close() == true)
		for _, op in ipairs(ops) do
			local ok, errmsg = pcall(socket[op], socket)
			assert(not ok)
			assert(errmsg == "attempt to use a closed socket")
		end
	end
end

return tests
