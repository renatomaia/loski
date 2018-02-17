local network = require "network"
local utils = require "test.utils"

local hosts = {
	localhost = { ["127.0.0.1"] = "ipv4" },
	["ip6-localhost"] = { ["::1"] = "ipv6" },
	["*"] = {
		["0.0.0.0"] = "ipv4",
		["::"] = "ipv6",
	},
	[false] = {
		["127.0.0.1"] = "ipv4",
		["::1"] = "ipv6",
	},
}
local servs = {
	ssh = 22,
	http = 80,
	[false] = 0,
	[54321] = 54321,
}
local scktypes = {
	datagram = true,
	stream = true,
	listen = true,
}
local addrtypes = {
	ipv4 = network.address("ipv4"),
	ipv6 = network.address("ipv6"),
}

for hostname, ips in pairs(hosts) do
	for servname, servport in pairs(servs) do
		if servname or (hostname and hostname ~= "*") then
			local next = assert(network.resolve(hostname or nil, servname or nil))
			local last = next.domain
			repeat
				local addr = assert(addrtypes[last])
				local other = last == "ipv4" and addrtypes.ipv6 or addrtypes.ipv4
				utils.testerror("wrong domain", next, other)
				local found, scktype = next(addr)
				assert(rawequal(found, addr))
				assert(ips[found.literal] == last)
				assert(found.port == servport)
				assert(scktypes[scktype] == true)
				assert(next.domain == nil or addrtypes[next.domain] ~= nil)
				last = next.domain
			until last == nil

			local next = assert(network.resolve(hostname or nil, servname or nil))
			repeat
				local found, scktype = assert(next())
				assert(ips[found.literal] ~= nil)
				assert(found.port == servport)
				assert(scktypes[scktype] == true)
			until next.domain == nil
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

utils.testerror("unknown mode char (got 'i')",
	network.resolve, "localhost", 0, "ipv6")
