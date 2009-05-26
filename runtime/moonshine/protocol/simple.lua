local Client   = require "moonshine.net.client"
local Protocol = require "moonshine.protocol.base"
local Simple   = Protocol:subclass()

-- later: local meta = Simple:meta()
-- meta:add_attribute("hostname", { required = true })
Simple:add_attribute('hostname', { required = true })
Simple:add_attribute('port',     { required = true })
Simple:add_attribute("client",   {
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

function Simple:make_tag(i)
	local hostname = split("%.", self:hostname())
	local tag

	if #hostname == 0 then
		tag = 'simple'
	elseif #hostname == 1 then
		tag = hostname[1]
	elseif #hostname == 2 then
		tag = hostname[1]
	elseif #hostname == 3 then
		tag = hostname[2]
	elseif #hostname > 3 then
		tag = hostname[1]
	end

	if i == 0 then
		return tag
	else
		return tag .. tostring(i+1)
	end
end

function Simple:__init()
	local client = Client:new(self:hostname(), self:port(), function(client, event, ...)
		self[ "on_" .. event](self, ...)
	end)
	self:client(client)
end

function Simple:on_connect()
end

function Simple:on_close()
end

function Simple:on_read(line)
end

function Simple:on_write()
end

function Simple:on_timeout()
end

return Simple
