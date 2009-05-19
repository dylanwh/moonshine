local Object = require "moonshine.object"
local Protocol = Object:new { }

function Protocol:__init()
	assert(self.host)
	assert(self.port)

	self.client = client:new(self.host, self.port, function(event, ...)
		self[ "on_" .. event](self, ...)
	end)
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

for i, name in ipairs { "read", "readn", "readline", "connect", "disconnect", "is_connected", "write" } do
	Protocol[name] = function(self, ...)
		return self.client[name](self.client, ...)
	end
end

return Protocol
