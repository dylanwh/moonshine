local prefs    = require "moonshine.prefs"
local Client   = require "moonshine.net.client"
local Object   = require "moonshine.object"
local Protocol = Object:subclass()

local NOTAG = "-NOTAG-"

-- later: local meta = Protocol:meta()
-- meta:add_attribute("hostname", { required = true })
Protocol:add_attribute('username', { is = "rw", default = prefs.username })
Protocol:add_attribute('tag',      { is = "rw" })

function Protocol:make_tag(i)
	assert(getmetatable(self))
	return 'protocol' .. tostring(i+1)
end

function Protocol:attach(env)
	assert(getmetatable(self))

	-- find unused tag, using self:make_tag()
	local tag = self:tag()

	if not tag or tag == NOTAG then
		local i = 0
		repeat
			tag = self:make_tag(i)
			i   = i + 1
		until not env[tag]
	end

	-- store protocol in the envrionment, using tag as key.
	env[tag] = self
	self:tag(tag)

	return tag
end

function Protocol:detach(env)
	assert(getmetatable(self))

	-- remove protocol from environment.
	env[self:tag()] = nil

	-- set tag to NOTAG, invalid tag constant.
	self:tag(NOTAG)
end

return Protocol
