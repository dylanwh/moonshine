local Client   = require "moonshine.net.client"
local Protocol = require "moonshine.protocol.base"
local Simple   = Protocol:clone()

-- later: local meta = Simple:meta()
-- meta:add_attribute("hostname", { required = true })
Simple:add_attribute('hostname')
Simple:add_attribute('port')
Simple:add_attribute('client')

do
	local names = {
		"read",
		"readn",
		"readline",
		"connect",
		"disconnect",
		"is_connected",
		"write"
	}

	for i, name in ipairs(name) do
		Simple[name] = function(self, ...)
			local client = self:client()
			return client[name](client, ...)
		end
	end
end

function Simple:__new()
	Protocol.__new(self)
	assert(self:hostname(), "hostname required")
	assert(self:port(),     "port required")

	local client = Client:new(self:hostname(), self:port(), function(client, event, ...)
		self[ "on_" .. event](self, ...)
	end)
	self:client(client)
end

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
