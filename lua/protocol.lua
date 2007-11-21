require "object"


-- A protocol should define the following methods:
-- * join(room)
-- * part(room)
-- * userlist(room)
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

