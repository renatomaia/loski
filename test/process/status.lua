local time = require "time"
local process = require "process"
local tests = require "test.process.utils"

do
	local script = tests.startscript{
		script = 'require("time").sleep(1)',
	}
	for i = 1, 3 do
		time.sleep(.3)
		assert(process.status(script.process) == "running")
	end
	time.sleep(.3)
	assert(process.status(script.process) == "dead")
	script:assertexit()
end

do
	local script = tests.startscript{
		script = 'require("time").sleep(1)',
	}
	time.sleep(.1)
	assert(process.status(script.process) == "running")
	assert(process.kill(script.process))
	time.sleep(.1)
	assert(process.status(script.process) == "dead")
	script:assertexit("aborted")
end
