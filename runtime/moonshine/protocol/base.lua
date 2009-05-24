local Client = require "moonshine.net.client"
local Object = require "moonshine.object"
local Protocol = Object:clone { }

function Protocol:connect()
	assert(type(self.hostname)   == 'string',   'hostname parameter required')
	assert(type(self.port)       == 'number',   'port parameter required')
	self.client   = Client:new(self.hostname, self.port, function(client, event, ...)
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
