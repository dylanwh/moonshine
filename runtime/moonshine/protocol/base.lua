local prefs = require "moonshine.prefs"

local Client = require "moonshine.net.client"
local Object = require "moonshine.object"
local Protocol = Object:clone()

-- later: local meta = Protocol:meta()
-- meta:add_attribute("hostname", { required = true })
Protocol:add_attribute('hostname', { required = true })
Protocol:add_attribute('port',     { required = true })
Protocol:add_attribute('username', { default = prefs.username })
Protocol:add_attribute("client",   {
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
	local client = Client:new(self:hostname(), self:port(), function(client, event, ...)
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

return Protocol
