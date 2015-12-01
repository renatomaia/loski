local network = require "network"
local utils = require "test.utils"

local hosts = {
	localhost = "127.0.0.1",
}
local servs = {
	ssh = 22,
	http = 80,
}

for servname, port in pairs(servs) do
	assert(network.getname(port) == servname)
	for hostname, ip in pairs(hosts) do
		local name, service = network.getname(network.address(ip, port))
		assert(name == hostname)
		assert(service == servname)
	end
end

assert(network.getname("www.tecgraf.puc-rio.br") == "webserver.tecgraf.puc-rio.br")

