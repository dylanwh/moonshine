require "protocol"

IRC = Protocol:clone { __type = "IRC" }

local function ircsplit(cmd)
  local t = {}
  for word, colon, start in cmd:split"%s+(:?)()" do
    t[#t+1] = word
    if colon == ":" then
      t[#t+1] = cmd:sub(start)
      break
    end
  end
  if string.sub(t[1], 1, 1) == ":" then
  	  t.prefix = string.sub(table.remove(t, 1), 2)
  end
  t.cmd = table.remove(t, 1)
  return t
end

function IRC:init()
	self.userlist = self:magic_table()
end

function IRC:connect(hostname, port)
	self.hostname = hostname or self.hostname
	self.port     = port     or self.port

	net.connect(self.hostname, self.port, self:callback "on_connect")

	connect_hook(self)
end

function IRC:disconnect()
	if self.handle then
		self.handle:close()
	end
	disconnect_hook(self)
end

function IRC:on_connect(fd, err)
	if fd then
		connected_hook(self, fd)
		self.reader = LineReader:new()
		self.handle = Handle:new(fd, self:callback "on_event")
		self:send("NICK %s", self.username)
		self:send('USER %s hostname servername :%s', self.username, 'Moonshine User')
	else
		connection_error_hook(err)
	end
end

function IRC:on_event(event, ...)
	if event == 'read' then
		local lines = self.reader:read(...)
		for i, line in ipairs(lines) do
			line = line:gsub("\r$", "")
			local msg = ircsplit(line)
			self:irc_event(msg)
		end
	elseif event == 'eof' then
		screen:debug("eof")
		self:shutdown()
	elseif event == 'hup' then
		screen:debug("hup")
		self:shutdown()
	elseif event == 'error' then
		local err = ...
		screen:debug("error: %1[%2]: %3", err.domain, err.code, err.message)
		self:shutdown()
	end
end

function IRC:irc_event(msg)
	local cmd = string.upper(msg.cmd)
	if self[cmd] and type(self[cmd]) == 'function' then
		self[cmd](self, msg)
	else
		screen:debug("prefix = %1, cmd = %2, args: %3", msg.prefix, msg.cmd, join(", ", msg))
	end
end

function IRC:send(format, ...)
	local line = string.format(format, ...) .. "\r\n"
	assert(#line <= 512)
	self.handle:write(line)
end

function IRC:join(room)
	if not room:match("^[#&]") then
		room = "#" .. room
	end
	self:send('JOIN %s', room)
end

function IRC:part(room)
	if not room:match("^[#&]") then
		room = "#" .. room
	end
	self:send('PART %s', room)
end

function IRC:usersof(room)
	if not room:match("^[#&]") then
		room = "#" .. room
	end
	self.userlist[room] = {}
	self:send('NAMES %s', room)
end

function IRC:msg(target, kind, msg)
	local name = target.name
	if target.type == 'room' then
		if not name:match("^[#&]") then
			name = "#" .. name
		end
		public_message_sent_hook(self, name, self.username, kind, msg)
	elseif target.type == 'user' then
		private_message_sent_hook(self, name, kind, msg)
	else
		screen:debug("Unknown target type: %1", target.type)
	end
	if kind == 'do' then
		msg = "\001ACTION "..msg.."\001"
	end
	self:send('PRIVMSG %s :%s', name, msg)
end

-- NICK response
IRC['433'] = function (self, msg)
	self.username = msg[2] .. "_"
	self:send("NICK %s", self.username)
	screen:debug("server: %|%1", msg[3])
end

-- NAMES response
IRC['353'] = function (self, msg)
	local names = msg[4]
	local room  = msg[3]
	for name in names:split(" ") do
		if name:len() > 0 then
			table.insert(self.userlist[room], name)
		end
	end
end

IRC['366'] = function (self, msg)
	local room = msg[2]
	userlist_hook(self, room, self.userlist[room])
end

function IRC:PRIVMSG(msg)
	local user = msg.prefix:match("(.+)!")
	local kind = 'say'
	local name, text = msg[1], msg[2]
	local ctcp = string.match(text, "^\001(.+)\001$")
	if ctcp then
		kind, text = string.match(ctcp, "^([A-Z]+) ?(.+)$")
	end
	if kind == "ACTION" then
		kind = "do"
	elseif kind == "VERSION" then
		kind = nil
		self:send('PRIVMSG %s :\001VERSION Moonshine %s\001', user, VERSION)
	end

	if kind then
		if string.sub(name, 1, 1) == '#' then
			public_message_hook(self, name, user, kind, text)
		else
			private_message_hook(self, user, kind, text)
		end
	end
end

function IRC:JOIN(msg)
	local user = msg.prefix:match("(.+)!")
	local room = msg[1]
	self.userlist[room] = {}
	join_hook(self, room, user)
end

function IRC:PART(msg)
	local user = msg.prefix:match("(.+)!")
	local room = msg[1]
	self.userlist[room] = nil
	part_hook(self, room, user)
end

function IRC:PING(msg)
	self:send("PING %s", msg[1])
end

function IRC:MODE(msg)
	-- ignore
end

function IRC:PONG(msg)
	-- ignore
end
