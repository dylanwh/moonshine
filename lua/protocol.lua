require "object"

-- A protocol should define the following methods:
-- * join(room)
-- * part(room)
-- * usersof(room)
-- * msg(target, kind, msg)
-- * connect(hostname, port)
-- * disconnect()
-- * canonize_name(type, name)
--
-- The following fields are optional:
-- * username (default: $USER)

Protocol = Object:clone {
	__type = "Protocol",
	username = os.getenv "USER",
}

function Protocol:canonize_name(type, name)
	return name:lower()
end

-- Returns a table which canonizes its keys according to the protocol's own
-- canonize_name() method. By default, this yields a case-insensitive table.
function Protocol:magic_table(type)
	local t = {}
	local mt = {}
	function mt.__index(t, key)
		return rawget(t, self:canonize_name(type, key))
	end
	function mt.__newindex(t, key, val)
		return rawset(t, self:canonize_name(type, key), val)
	end
	setmetatable(t, mt)
	return t
end
