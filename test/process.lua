local time = require "time"
local process = require "process"

local nop = 100
local proc = {}

for i=1, nop do
	io.write("P",i," ... ")
	io.flush()
	local t = time.now()
	proc[i] = assert(process.create{
		execfile = "lua",
		arguments = {"-ltime", "-e", "while true do io.write(' ',os.getenv('PROCID')) io.flush() time.sleep(1) end"},
		environment = {
			PATH = os.getenv("PATH"),
			LUA_INIT = os.getenv("LUA_INIT"),
			LUA_PATH = os.getenv("LUA_PATH"),
			LUA_CPATH = os.getenv("LUA_CPATH"),
			PROCID = i-1,
		},
	})
	io.write("OK (",time.now()-t,")\n")
	io.flush()
end

assert(process.create("lua", "-e", [[print("I'll become a zombie")]]))

for i=1, nop do
	io.write(".")
	io.flush()
	time.sleep(.1)
	assert(proc[i]:status() == "running")
end

for i=1, nop do
	assert(proc[i]:kill())
end

time.sleep(1)

for i=1, nop do
	assert(proc[i]:status() == "dead")
end

io.write("\nCompleted!")
io.read()
