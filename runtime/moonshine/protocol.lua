local M = {}
local TAGS = {}

function M.connect(name, ...)
	local log    = require "moonshine.log"
	local Class  = require("moonshine.protocol." .. name)
	local protocol = Class:new(...)
	local tag    = protocol:attach(TAGS)

	protocol:connect()

	return tag
end

local METHODS = { "join", "part", "quit", "message", "disconnect" }

for _, name in ipairs(METHODS) do
	M[name] = function(tag, ...)
		local obj  = TAGS[tag]
		local func = self[name]
		return func(obj, ...)
	end
end

return M
