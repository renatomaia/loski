local time = require "time"
local process = require "process"

local tests = require "test.utils"

tests.luabin = "lua"
do
	local i = -1
	while arg[i] ~= nil do
		tests.luabin = arg[i]
		i = i-1
	end
end

function tests.writeto(path, ...)
	local file = assert(io.open(path, "w"))
	assert(file:write(...))
	assert(file:close())
end

function tests.readfrom(path)
	local file = io.open(path, "r")
	if file ~= nil then
		local data = assert(file:read("*a"))
		assert(file:close())
		return data
	end
end

do
	local scriptfile = os.tmpname()
	local successfile = os.tmpname()

	local function fillenv(env)
		if env ~= nil then
			env.PATH = os.getenv("PATH")
			env.LUA_INIT = os.getenv("LUA_INIT")
			env.LUA_PATH = os.getenv("LUA_PATH")
			env.LUA_CPATH = os.getenv("LUA_CPATH")
		end
		return env
	end

	function tests.startscript(info, ...)
		local command = tests.luabin
		local script = info
		if type(info) == "table" then
			script = info.script
			local arguments = {scriptfile}
			if info.arguments ~= nil then
				for index, argval in ipairs(info.arguments) do
					arguments[index+1] = argval
				end
			end
			command = {
				execfile = command,
				runpath = info.runpath,
				environment = fillenv(info.environment),
				arguments = arguments,
				stdin = info.stdin,
				stdout = info.stdout,
				stderr = info.stderr,
			}
		end
		tests.writeto(scriptfile, [[
			local function main(...) ]], script, [[ end
			local exitval = main(...)
			local file = assert(io.open("]],successfile,[[", "w"))
			assert(file:write("SUCCESS!"))
			assert(file:close())
			os.exit(exitval)
		]])
		os.remove(successfile)
		local proc = assert(process.create(command, scriptfile, ...))
		return {
			process = proc,
			assertexit = function (self, expected)
				while process.status(proc) ~= "dead" do
					time.sleep(.1)
				end
				assert(os.remove(scriptfile))
				local success = tests.readfrom(successfile)
				os.remove(successfile)
				if type(expected) == "string" then
					tests.testerrmsg(expected, process.exitval(proc))
				else
					assert(success == "SUCCESS!")
					assert(process.exitval(proc) == (expected or 0))
				end
			end
		}
	end

	function tests.runscript(...)
		return tests.startscript(...):assertexit()
	end
end

return tests