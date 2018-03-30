local memory = require "memory"
local filesys = require "filesys"
local tests = require "test.utils"

for _, mode in ipairs{ "", "p", "a", "t", "n", "N", "patn", "patN" } do
	tests.testerror("invalid operation", filesys.open, "deleteme.dat", mode)
end

do
	local path = os.tmpname()

	local fread = assert(filesys.open(path, "r"))
	local fwrite = assert(filesys.open(path, "w"))
	local mem = memory.create(8)

	tests.testerror("invalid operation", fread.write, fread, "contents")
	tests.testerror("invalid operation", fwrite.read, fwrite, mem)

	assert(fread:read(mem) == 0)
	assert(memory.diff(mem, string.rep("\0", 8)) == nil)
	assert(fwrite:write("contents") == 8)
	assert(fread:read(mem) == 8)
	assert(memory.diff(mem, "contents") == nil)
	assert(fread:read(mem) == 0)
	assert(memory.diff(mem, "contents") == nil)

	assert(fwrite:close())
	assert(fread:close())
	assert(os.remove(path))
end
