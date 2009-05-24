local api = require "moonshine.object.api"
local prefs = require "moonshine.prefs"

local Client = require "moonshine.net.client"
local Object = require "moonshine.object"
local Protocol = Object:clone()

api.add_attribute(Protocol, 'hostname', { required = true })
api.add_attribute(Protocol, 'port',     { required = true })
api.add_attribute(Protocol, 'username', { default = prefs.username})
api.add_attribute(Protocol, "client",   {
	handles = {
		"read",
		"readn",
		"readline",
		"connect",
		"disconnect",
		"is_connected",
		"write"
	}
})

function Protocol:__init()
	assert(type(self:hostname())   == 'string',   'hostname parameter required')
	assert(type(self:port())       == 'number',   'port parameter required')
	local client = Client:new(self.hostname, self.port, function(client, event, ...)
		self[ "on_" .. event](self, ...)
	end)
	self:client(client)
	self:connect()
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
