local M = {}

-- protocol registry
local protocols  = {}

function M.register(name, object)
	-- TODO: validate object's interface.
	registry[name] = object
end

function M.connect(name, params)
	local protocol = protocols[ name ]
	assert(protocol, "unknown protocol name")
	local tag = protocol:new(params)

	tag:connect()

	return tag
end


