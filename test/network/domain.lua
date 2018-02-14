local network = require "network"
local tests = require "test.network.utils"

local address = {}
for domain, data in pairs{ipv4="5.5.5.5", ipv6="::ffff:0505:0505"} do
	address[domain] = network.address(domain, data, 5)
end

for domain in pairs(address) do
	for _, kind in ipairs{"datagram", "stream", "listen"} do
		local socket = network.socket(kind, domain)
		for other, addr in pairs(address) do
			if other ~= domain then
				tests.testerror("wrong domain", socket.bind, socket, addr)
				tests.testerror("wrong domain", socket.getaddress, socket, "this", addr)
				if kind == "listen" then
					tests.testerror("wrong domain", socket.accept, socket, addr)
				elseif kind == "stream" then
					tests.testerror("wrong domain", socket.connect, socket, addr)
				elseif kind == "datagram" then
					tests.testerror("wrong domain", socket.send, socket, "blah", 1, -1, addr)
					tests.testerror("wrong domain", socket.receive, socket, 10, "", addr)
				end
			end
		end
	end
end
