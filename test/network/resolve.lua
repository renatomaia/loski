local network = require "network"
local utils = require "test.utils"

local hosts = {
	localhost = {
		["::1"] = true,
		["127.0.0.1"] = true,
	},
	["*"] = {
		["::"] = true,
		["0.0.0.0"] = true,
	},
	[false] = {
		["::1"] = true,
		["127.0.0.1"] = true,
	},
}
local servs = {
	ssh = 22,
	http = 80,
	[false] = 0,
}
local scktypes = {
	datagram = true,
	connection = true,
	listen = true,
}
local addr = network.address()

for hostname, ips in pairs(hosts) do
	for servname, servport in pairs(servs) do
		if servname or (hostname and hostname ~= "*") then
			local last
			local next = assert(network.resolve(hostname or nil, servname or nil))
			for found, scktype, more in next, addr do
				assert(rawequal(found, addr))
				assert(last == nil or last == true)
				assert(ips[found.literal] == true)
				assert(found.port == servport)
				assert(scktypes[scktype] == true)
				assert(type(more) == "boolean")
				last = more
			end
			assert(last == false)
		end
	end
end


utils.testerror("name or service must be provided",
	network.resolve)
utils.testerror("name or service must be provided",
	network.resolve, nil)
utils.testerror("name or service must be provided",
	network.resolve, nil, nil)

utils.testerror("service must be provided for '*'",
	network.resolve, "*")
utils.testerror("service must be provided for '*'",
	network.resolve, "*")
utils.testerror("service must be provided for '*'",
	network.resolve, "*", nil)

do return  end

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
