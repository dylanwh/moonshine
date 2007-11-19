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
  return t
end


function IRC:connect(hostname, port)
	self.hostname = hostname or self.hostname
	self.port     = port     or self.port

	self.tag = self:make_tag()
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
	if msg[1] == "433" then
		self.username = msg[3] .. "_"
		self:send("NICK %s", self.username)
		screen:debug("server: %|%1", msg[4])
	elseif msg[1] == 'PRIVMSG' then
		local user = msg.prefix:match("(.+)!")
		if string.sub(msg[2], 1, 1) == '#' then
			public_message_hook(self, string.sub(msg[2], 2), user, 'say', msg[3])
		else
			private_message_hook(self, user, 'say', msg[3])
		end
	elseif msg[1] == 'JOIN' then
		local user = msg.prefix:match("(.+)!")
		join_hook(self, string.sub(msg[2], 2), user)
	elseif msg[1] == 'PART' then
		local user = msg.prefix:match("(.+)!")
		part_hook(self, string.sub(msg[2], 2), user)
	else
		screen:debug("prefix = %1, cmd = [%2]", msg.prefix, join("|", msg))
	end
end

function IRC:send(format, ...)
	local line = string.format(format, ...) .. "\r\n"
	assert(#line <= 512)
	self.handle:write(line)
end

function IRC:join(room) self:send('JOIN #%s', room) end
function IRC:part(room) self:send('PART #%s', room) end

function IRC:userlist(room)
	--self:send('USERSOF', room)
end

function IRC:msg(target, kind, msg)
	local name
	if target.type == 'room' then
		name = "#" .. target.name
		public_message_sent_hook(self, target.name, self.username, kind, msg)
	elseif target.type == 'user' then
		name = target.name
		private_message_sent_hook(self, target.name, kind, msg)
	else
		screen:debug("Unknown target type: %1", target.type)
	end
	if kind == 'say' then
		self:send('PRIVMSG %s :%s', name, msg)
	else
		screen:debug("IRC cannot handle message type: %1", kind)
	end
end
