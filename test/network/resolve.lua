local network = require "network"
local utils = require "test.utils"

local hosts = {
	localhost = "127.0.0.1",
}
local servs = {
	ssh = 22,
	http = 80,
}
local scktypes = {
	datagram = true,
	connection = true,
}
local addr = network.address()
for hostname, hostip in pairs(hosts) do
	for servname, servport in pairs(servs) do
		local last
		local next = assert(network.resolve(hostname, servname))
		for found, scktype, more in next, addr do
			assert(rawequal(found, addr))
			assert(last == nil or last == true)
			assert(found.literal == hostip)
			assert(found.port == servport)
			assert(scktypes[scktype] == true)
			assert(type(more) == "boolean")
			last = more
		end
		assert(last == false)
	end
end

do return end

	-- get first result only
	addr, scktype = network.resolve("www.google.com", "http")()

	-- collect all results
	list = {}
	for addr, scktype in network.resolve("www.google.com", "http") do
		list[#list+1] = { addr, scktype }
	end

	-- filling existing addreses object with the results
	next = network.resolve("www.google.com", "http")
	repeat until not select(3, next(getSomeAddressObject()))
