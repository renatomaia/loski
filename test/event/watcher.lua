local event = require "event"
local network = require "network"
local time = require "time"
local testutils = require "test.utils"

do
	local watcher = assert(event.watcher())
	testutils.testerror("invalid operation", watcher.wait, watcher)
end

do
	local watcher = assert(event.watcher())
	assert(watcher:close())
	testutils.testerror("attempt to use a closed event watcher",
	                    watcher.set, watcher, io.stdin, "r")
	testutils.testerror("attempt to use a closed event watcher",
	                    watcher.wait, watcher)
	testutils.testerror("attempt to use a closed event watcher",
	                    watcher.close, watcher)
end

do
	local watcher = assert(event.watcher())
	testutils.testerror("unknown event (got 'e')",
	                    watcher.set, watcher, io.stdout, "read")
	local types = {
		nil,
		false,
		true,
		3232235776,
		"text",
		table,
		print,
		coroutine.running(),
	}
	for _, type in ipairs(types) do
		testutils.testerror("invalid watchable object",
		                    watcher.set, watcher, value, "r")
	end
	local dgm = assert(network.socket("datagram"))
	local con = assert(network.socket("stream"))
	local lst = assert(network.socket("listen"))
	assert(watcher:set(dgm, "r"))
	assert(watcher:set(con, "w"))
	assert(watcher:set(lst, "wr"))

	local dgm = assert(network.socket("datagram"))
	assert(dgm:close())
	testutils.testerrmsg("closed", watcher:set(dgm, "r"))
end

do
	local watcher = assert(event.watcher())
	local weak = setmetatable({}, { __mode = "v" })
	weak.s1 = assert(network.socket("stream"))
	weak.s2 = assert(network.socket("stream"))
	assert(watcher:set(weak.s1, "rw"))
	assert(watcher:set(weak.s2, "wr"))
	collectgarbage()
	assert(weak.s1 ~= nil)
	assert(weak.s2 ~= nil)
	assert(watcher:set(weak.s1))
	collectgarbage()
	assert(weak.s1 == nil)
	assert(weak.s2 ~= nil)
	assert(watcher:close())
	collectgarbage()
	assert(weak.s1 == nil)
	assert(weak.s2 == nil)
end
