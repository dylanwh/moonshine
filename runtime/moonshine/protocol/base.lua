local prefs    = require "moonshine.prefs"
local Client   = require "moonshine.net.client"
local Object   = require "moonshine.object"
local Protocol = Object:clone()

local NOTAG = "-NOTAG-"

Protocol:add_attribute('username')
Protocol:add_attribute('tag')

function Protocol:__new()
	assert(self:username(), "username required")
end

function Protocol:make_tag(i)
	assert(getmetatable(self))
	return 'protocol' .. tostring(i+1)
end

function Protocol:attach(env)
	assert(getmetatable(self))

	-- find unused tag, using self:make_tag()
	local i = 0
	repeat
		tag = self:make_tag(i)
		i   = i + 1
	until not env[tag]

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

-- trigger a hook, with tag context.
function Protocol:trigger(name, ...)
	run_hook(name, self:tag(), ...)
end

return Protocol
