local network = require "network"
local utils = require "test.utils"

local hosts = {
	localhost = { ipv4 = "127.0.0.1" },
	["ip6-localhost"] = { ipv6 = "::1" },
}
local servs = {
	ssh = 22,
	http = 80,
}

for servname, port in pairs(servs) do
	assert(network.getname(port) == servname)
	for hostname, ips in pairs(hosts) do
		for domain, ip in pairs(ips) do
			local addr = network.address(domain, ip, port)
			local name, service = network.getname(addr)
			assert(name == hostname)
			assert(service == servname)
		end
	end
end

assert(network.getname("www.tecgraf.puc-rio.br") == "webserver.tecgraf.puc-rio.br")

