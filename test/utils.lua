local tests = {}

function tests.testerror(expected, func, ...)
	local ok, errmsg = pcall(func, ...)
	assert(not ok)
	assert(string.find(errmsg, expected, 1, true), errmsg)
end

function tests.testerrmsg(expected, res, errmsg)
	assert(res == nil)
	assert(string.find(errmsg, expected, 1, true), errmsg)
end

return tests
