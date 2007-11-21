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
--
-- The following fields are read-only:
-- * tag

Protocol = Object:clone {
	__type = "Protocol",
	username = os.getenv "USER",
}

function Protocol:canonize_name(type, name)
	return name:lower()
end

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


