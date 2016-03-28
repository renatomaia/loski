local process = require "process"
local tests = require "test.process.utils"

do
	local argtests = {
		{
			script = [[
			assert(select("#", ...) == 1)
			assert(select(1, ...) == "f1")
			]],
			arguments = {"f1"},
		},
		{
			script = [[
			assert(select("#", ...) == 3)
			assert(select(1, ...) == "f1")
			assert(select(2, ...) == "f2")
			assert(select(3, ...) == "f3")
			]],
			arguments = {"f1", "f2", "f3"},
		},
		{
			script = [[
			assert(select("#", ...) == 1)
			assert(select(1, ...) == "f1 f2 f3")
			]],
			arguments = {"f1 f2 f3"},
		},
		{
			script = [[
			assert(select("#", ...) == 1)
			assert(select(1, ...) == '"f1", "f2", "f3"')
			]],
			arguments = {'"f1", "f2", "f3"'},
		},
		{
			script = [[
			assert(select("#", ...) == 1)
			assert(select(1, ...) == "f1")
			]],
			arguments = {"f1\0f2\0f3"},
		}
	}

	for _, case in ipairs(argtests) do
		tests.runscript(case.script, table.unpack(case.arguments))
		tests.runscript(case)
	end
end

do
	for index, code in ipairs{ 0, 1, 2, 3, 127, 128, 255 } do
		local script = "return "..code
		tests.startscript(script):assertexit(code)
		tests.startscript{ script = script }:assertexit(code)
	end
end

do
	tests.runscript(string.format('assert(os.getenv("HOME") == %q)', os.getenv("HOME")))

	tests.runscript{
		script = [[
			assert(os.getenv("ENV_VAR_01") == "environment variable 01")
			assert(os.getenv("ENV_VAR_02") == "environment variable 02")
			assert(os.getenv("ENV_VAR_03") == "environment variable 03")
			assert(os.getenv("HOME") == nil)
		]],
		environment = {
			ENV_VAR_01 = "environment variable 01",
			ENV_VAR_02 = "environment variable 02",
			ENV_VAR_03 = "environment variable 03",
		},
	}

	tests.runscript{
		script = [[
			assert(string.match(os.getenv("ENV_VAR"), "^environment variable 0[123]$"))
			assert(os.getenv("ENV_VAR_01") == nil)
			assert(os.getenv("ENV_VAR_02") == nil)
			assert(os.getenv("ENV_VAR_03") == nil)
			assert(os.getenv("HOME") == nil)
		]],
		environment = {
			["ENV_VAR\0_01"] = "environment variable 01",
			["ENV_VAR\0_02"] = "environment variable 02",
			["ENV_VAR\0_03"] = "environment variable 03",
		},
	}

	tests.testerror(
		"bad name '=ENV' in field 'environment' (must be a string without '=')",
		process.create,
		{ execfile = tests.luabin, environment = { ["=ENV"] = "illegal" } })

	tests.runscript{
		script = 'assert(os.getenv("HOME") == "My Home!")',
		environment = { ["HOME"] = "My Home!" },
	}
end

do
	local tempfile = os.tmpname()
	local outcases = {
		"Hello, World!",
		"\0\1\2\3",
		[[
			one single line
			another additional line
			yet even another final line
		]],
	}
	for _, case in ipairs(outcases) do
		for _, output in ipairs{"stdout", "stderr"} do
			local file = io.open(tempfile, "w")
			tests.runscript{
				script = string.format('io.%s:write(%q)', output, case),
				[output] = file,
			}
			file:close()
			assert(tests.readfrom(tempfile) == case)
			os.remove(tempfile)
		end

		tests.writeto(tempfile, case)
		local file = io.open(tempfile, "r")
		tests.runscript{
			script = string.format('assert(io.stdin:read("*a") == %q)', case),
			stdin = file,
		}
		file:close()
		os.remove(tempfile)
	end
end

do
	local function teststdfiles(info)
		local infile = os.tmpname()
		local outfile = os.tmpname()
		local errfile = os.tmpname()
		tests.writeto(infile, info.input)
		local stderr = io.open(errfile, "w")
		local stdout = io.open(outfile, "w")
		local stdin = io.open(infile, "r")
		tests.runscript{
			script = info.script,
			stdin = stdin,
			stdout = stdout,
			stderr = stderr,
		}
		stdin:close()
		stdout:close()
		stderr:close()
		assert(tests.readfrom(outfile) == info.output)
		assert(tests.readfrom(errfile) == info.errors)
		os.remove(infile)
		os.remove(outfile)
		os.remove(errfile)
	end

	teststdfiles{
		script = [[
			assert(io.stdin:read("*a") == "stdin")
			assert(io.stdout:write("stdout"))
			assert(io.stderr:write("stderr"))
		]],
		input = "stdin",
		output = "stdout",
		errors = "stderr",
	}

	teststdfiles{
		script = [[
			local tests = require "test.process.utils"
			tests.runscript{
				script = [=[
					assert(io.stdin:read("*a") == "stdin")
					assert(io.stdout:write("stdout"))
					assert(io.stderr:write("stderr"))
				]=],
				stdout = io.stderr,
				stderr = io.stdout,
			}
		]],
		input = "stdin",
		output = "stderr",
		errors = "stdout",
	}

	teststdfiles{
		script = [[
			local tests = require "test.process.utils"
			tests.runscript{
				script = [=[
					assert(io.stdin:write("stdin") == nil)  -- TODO: why?
					assert(io.stdout:read("*a") == nil)     -- TODO: why?
					assert(io.stderr:write("stderr"))
				]=],
				stdin = io.stderr,
				stdout = io.stdin,
				stderr = io.stdout,
			}
		]],
		input = "stdout",
		output = "stderr",
		errors = "", -- TODO: shouldn't be "stdin"?
	}
end

do
	tests.runscript{
		script = [[
			local data = assert(io.open("testall.lua")):read("*a")
			assert(string.find(data, 'print("OK")', 1, true))
		]],
		runpath = "test",
	}

	tests.runscript{
		script = 'assert(io.open("deleteme.tmp", "w")):write("Delete me now!")',
		runpath = "/tmp",
	}
	assert(tests.readfrom("/tmp/deleteme.tmp") == "Delete me now!")
	os.remove("/tmp/deleteme.tmp")
end
