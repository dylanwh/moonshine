require "object"
require "server"

Haver = Object:new()

function Haver:init()
	self.hostname = self.hostname or 'chat.haverdev.org'
	self.port     = self.port or 7575
	self.username = self.username or os.getenv('USER')

	net.connect(self.hostname, self.port, self:callback "on_connect")

	self.tag = server_tag(self.hostname)
	servers[self.tag] = self

	self.rooms = {}
	self.users = {}
	screen:debug("Connecting to %1 (%2:%3)", self.tag, self.hostname, self.port)
end

function Haver:shutdown()
	if self.handle then
		self.handle:close()
	end

	servers[self.tag] = nil
end

function Haver:on_connect(fd, err)
	if fd then
		screen:debug("Connected to %1", self.tag)
		self.reader = LineReader:new()
		self.handle = Handle:new(fd, self:callback "on_event")
		self:send("HAVER", "Moonshine/"..VERSION)
	else
		self:shutdown()
	end
end

function Haver:on_event(event, ...)
	if event == 'read' then
		local lines = self.reader:read(...)
		for i, line in ipairs(lines) do
			local msg = split("\t", line:gsub("\r$", ""))
			local cmd = table.remove(msg, 1):upper()
			if self[cmd] then
				self[cmd](self, unpack(msg))
			else
				screen:debug("Unknown command: %1 (%2)", cmd, join(", ", {...}))
			end
		end
	elseif event == 'eof' then
		screen:debug("eof")
		self:shutdown()
	elseif event == 'hup' then
		screen:debug("hup")
		self:shutdown()
	elseif event == 'error' then
		local err = ...
		screen:debug("error: %1[%2]: %3", err,domain, err,code, err.message)
		self:shutdown()
	end
end

function Haver:send(...)
	if self.handle then
		self.handle:write( join("\t", {...}) .. "\n" )
	end
end

function Haver:join(room) self:send('JOIN', room) end
function Haver:part(room) self:send('PART', room) end

function Haver:msg(target, kind, msg)
	local cmd
	if target.type == 'room' then
		cmd = 'IN'
		public_message_sent_hook(self, target.name, self.username, kind, msg)
	elseif target.type == 'user' then
		cmd = 'TO'
		private_message_sent_hook(self, target.name, kind, msg)
	else
		screen:debug("Unknown target type: %1", target.type)
	end
	self:send(cmd, target.name, kind, msg)
end

function Haver:userlist(target)
	if target.type == 'room' then
		self:send('USERSOF', target.name)
	else
		screen:debug("Unknown target type: %1", target.type)
	end
end

function Haver:HAVER(host, version, extensions)
	self.server_version = version
	self.extensions = split(",", extensions)
	
	screen:debug("trying to log in as %1", self.username)
	self:send('IDENT', self.username)
end

function Haver:HELLO(username, address)
	screen:debug("Logged in as %1", username)
end

function Haver:IN(room, user, type, msg)
	if not (self.username == user) then
		public_message_hook(self, room, user, type, msg)
	end
end

function Haver:FROM(user, type, msg)
	private_message_hook(self, user, type, msg)
end

function Haver:JOIN(room, user)
	join_hook(self, room, user)
end

function Haver:PART(room, user, ...)
	part_hook(self, room, user)
end

function Haver:PING(token)
	self:send('PONG', token)
end

function Haver:USERSOF(room, ...)
	local users = {...}
	userlist_hook(self, room, users)
end
