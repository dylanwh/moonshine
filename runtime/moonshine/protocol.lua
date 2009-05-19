local Client = require "moonshine.net.client"
local Object = require "moonshine.object"
local Protocol = Object:new { }

function Protocol:connect(hostname, port)
	ensure(type(hostname)   == 'string',   'hostname parameter required')
	ensure(type(port)       == 'number',   'port parameter required')
	self.hostname = hostname
	self.port     = port
	self.client   = Client:new(self.hostname, self.port, function(client, event, ...)
		emit("irc event " .. event)
		self[ "on_" .. event](self, ...)
	end)
	self.client:connect()
end

function Protocol:on_connect()
end

function Protocol:on_close()
end

function Protocol:on_read(line)
end

function Protocol:on_write()
end

function Protocol:on_timeout()
end

for i, name in ipairs { "read", "readn", "readline", "disconnect", "is_connected", "write" } do
	Protocol[name] = function(self, ...)
		return self.client[name](self.client, ...)
	end
end

return Protocol
