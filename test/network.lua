local time = require "time"
local network = require "network"
local process = require "process"

local port = ...
local szlen = 4
local msgfmt = "%"..szlen.."d%s"

local server = assert(process.create("lua", "-e", [[
local network = require "network"

local msgfmt = ]]..string.format("%q", msgfmt)..[[

local port = assert(network.socket("listen"))
assert(port:bind("*", ]]..port..[[))
assert(port:listen())
local conn = assert(port:accept())
local size = assert(conn:receive(]]..szlen..[[))
local data = assert(conn:receive(size))
local result = assert(loadstring(data))()
local msg = msgfmt:format(#result, result)
assert(assert(conn:send(msg)) == #msg)
assert(conn:close())
assert(port:close())
]]))

time.sleep(1)

local conn = assert(network.socket("connection"))
-- change default values
assert(conn:setoption("blocking", false))
assert(conn:setoption("reuseaddr", true))
assert(conn:setoption("dontroute", true))
assert(conn:setoption("keepalive", true))
assert(conn:setoption("linger", 10))
assert(conn:setoption("nodelay", true))
assert(conn:setoption("broadcast", true))
-- check changed values
assert(conn:getoption("blocking") == false)
assert(conn:getoption("reuseaddr") == true)
assert(conn:getoption("dontroute") == true)
assert(conn:getoption("keepalive") == true)
assert(conn:getoption("linger") == 10)
assert(conn:getoption("nodelay") == true)
assert(conn:getoption("broadcast") == true)
-- restore default values
assert(conn:setoption("blocking", true))
assert(conn:setoption("reuseaddr", false))
assert(conn:setoption("dontroute", false))
assert(conn:setoption("keepalive", false))
assert(conn:setoption("linger", 0))
assert(conn:setoption("nodelay", false))
assert(conn:setoption("broadcast", false))
-- check restored values
assert(conn:getoption("blocking") == true)
assert(conn:getoption("reuseaddr") == false)
assert(conn:getoption("dontroute") == false)
assert(conn:getoption("keepalive") == false)
assert(conn:getoption("linger") == 0)
assert(conn:getoption("nodelay") == false)
assert(conn:getoption("broadcast") == false)
-- perform other tests
assert(conn:connect("localhost", port))
local code = "return _VERSION"
local msg = msgfmt:format(#code, code)
assert(assert(conn:send(msg)) == #msg)
local size = assert(conn:receive(szlen))
local data = assert(conn:receive(size))
assert(data == _VERSION)
