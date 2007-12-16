require "protocol"

IRC = Protocol:clone { __type = "IRC" }

-- Until we have real color support, this'll at least get the color codes
-- out of the way.
local function stripcolors(line)
  -- Bold, underline, reverse/italic, reset
  line = line:gsub("[\002\037\026\017]", "")
  line = line:gsub("\003%d+,%d+", "")
  line = line:gsub("\003%d+", "")
  line = line:gsub("\003", "")
  return line
end

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
	self.rooms = self:magic_table()
end

-- Because of IRC's scandanavian origin, the characters {}| are considered to be
-- the lower case equivalents of the characters []\, respectively. This is a
-- critical issue when determining the equivalence of two nicknames.
function IRC:canonize_name(type, name)
	name = name:lower()
	name = name:gsub("[%{%}%|]", function (c)
		return string.char(string.byte(x) - 32)
	end)
	return name
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
	if event == 'input' then
		local str = self.handle:read()
		if str then
			for line in self.reader:read(str) do
				line = line:gsub("\r$", "")
				local msg = ircsplit(line)
				self:irc_event(msg)
			end
		else
			screen:debug("eof")
			self:disconnect()
		end
	elseif event == 'error' then
		local errtype, err = ...
		if errtype == 'write' then
			screen:debug("write error: %1[%2]: %3", err.domain, err.code, err.message)
		else
			screen:debug("error: %1", errtype)
		end
		self:disconnect()
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
	self.rooms[room].users = {}
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
			table.insert(self.rooms[room].users, name)
		end
	end
end

IRC['366'] = function (self, msg)
	local room = msg[2]
	userlist_hook(self, room, self.rooms[room].users)
end

IRC['332'] = function (self, msg)
	local room, topic = msg[2], msg[3]
	topic_hook(self, room, topic)
end



function IRC:PRIVMSG(msg)
	local user = msg.prefix:match("(.+)!")
	local kind = 'say'
	local name, text = msg[1], msg[2]
	local ctcp = string.match(text, "^\001(.+)\001$")
	if ctcp then
		kind, text = string.match(ctcp, "^([A-Z]+) ?(.-)$")
		screen:debug("CTCP: %1 (%2)", kind, text)
	end
	if kind == "ACTION" then
		kind = "do"
	elseif kind == "VERSION" then
		kind = nil
		self:send('NOTICE %s :\001VERSION Moonshine %s\001', user, VERSION)
	end

    text = stripcolors(text)
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
	self.rooms[room] = {
		users = {},
		topic = nil,
	}
	join_hook(self, room, user)
end

function IRC:PART(msg)
	local user = msg.prefix:match("(.+)!")
	local room = msg[1]
	self.rooms[room] = nil
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
