local event = require "event"
local process = require "process"
local time = require "time"
local testutils = require "test.utils"

do
	local watcher = assert(event.watcher())
	local child = assert(process.create("sleep", 1))
	testutils.testerror("unsupported", watcher.set, watcher, child, "w")
	testutils.testerror("invalid operation", watcher.wait, watcher)
end

do
	local watcher = assert(event.watcher())
	local child = assert(process.create("sleep", 1))

	assert(watcher:set(child, "r"))
	testutils.testerrmsg("unfulfilled", watcher:wait(0))
	local start = time.now()
	testutils.testerrmsg("unfulfilled", watcher:wait(.5))
	assert(time.now() - start > .5)

	local events = assert(watcher:wait())
	assert(events[child] == "r")
	assert(process.exitval(child) == 0)

	assert(watcher:set(child, nil))
	testutils.testerrmsg("closed", watcher:set(child, "r"))
	testutils.testerror("invalid operation", watcher.wait, watcher)
end

do
	local watcher = assert(event.watcher())
	local child1 = assert(process.create("sleep", 1))
	local child2 = assert(process.create("sleep", 2))

	assert(watcher:set(child2, "r"))
	local events = assert(watcher:wait())
	assert(events[child1] == nil)
	assert(events[child2] == "r")
	assert(process.exitval(child2) == 0)
	local events = assert(watcher:wait(0))
	assert(events[child1] == nil)
	assert(events[child2] == "r")
	assert(watcher:set(child2, nil))

	testutils.testerror("invalid operation", watcher.wait, watcher)
end

do
	local watcher = assert(event.watcher())
	local child1 = assert(process.create("sleep", 1))
	local child2 = assert(process.create("sleep", 1))

	assert(watcher:set(child1, "r"))
	assert(watcher:set(child2, "r"))
	assert(watcher:set(child1, nil))
	time.sleep(2)
	assert(process.exitval(child2) == 0)
	assert(watcher:set(child2, nil))

	testutils.testerror("invalid operation", watcher.wait, watcher)
end

do
	local watcher = assert(event.watcher())
	local child = assert(process.create("sleep", 0))

	time.sleep(1)

	testutils.testerrmsg("closed", watcher:set(child, "r"))
end

do
	local w1 = assert(event.watcher())
	local w2 = assert(event.watcher())
	local child = assert(process.create("sleep", 1))

	assert(w1:set(child, "r"))
	assert(w2:set(child, "r"))
	local e1 = assert(w1:wait())
	local e2 = assert(w2:wait())
	assert(e1[child] == "r")
	assert(e2[child] == "r")
	assert(process.exitval(child) == 0)
end
