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


function Protocol:connect(hostname, port)
	self.hostname = hostname or self.hostname
	self.port     = port     or self.port

	connect_hook(self)
	net.connect(self.hostname, self.port, self:callback "on_connect")
end

function Protocol:disconnect()
	if self.handle then
		self.handle:close()
	end
	disconnect_hook(self)
end

function Protocol:on_connect(fd, ...)
	if fd then
		self.reader = LineReader:new()
		self.handle = Handle:new(fd, self:callback "on_event")
		connected_hook(self, fd)
	else
		return connection_error_hook(...)
	end
end

function Protocol:on_event(event, errtype, errmsg)
	if event == 'input' then
		local str = self.handle:read()
		if str then
			for line in self.reader:lines(str) do
				self:on_readline(line)
			end
		else
			screen:debug("eof")
			self:disconnect()
		end
	elseif event == 'error' then
		if errtype == 'write' then
			screen:debug("write error: %1", errmsg)
		else
			screen:debug("error: %1", errtype)
		end
		self:disconnect()
	end
end


