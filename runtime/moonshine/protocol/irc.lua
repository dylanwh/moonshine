local Protocol = require "moonshine.protocol.base"

local IRC = Protocol:subclass()

function IRC:write(msg)
	assert(#msg <= 510)
	return Protocol.write(self, msg .. "\r\n")
end

function IRC:send(fmt, ...)
	self:write(string.format(fmt, ...))
end

function IRC:on_connect()
	self:send("NICK %s", self:username())
	self:send('USER %s hostname servername :%s', self:username(), 'Moonshine User')
	run_hook('connected', self)
	self:readline()
end

local numerics = require "moonshine.protocol.irc.numerics"
local function ircsplit(line)
  	local t = {}
  	for word, colon, start in line:split"%s+(:?)()" do
    	t[#t+1] = word
    	if colon == ":" then
      		t[#t+1] = line:sub(start)
      		break
    	end
  	end
  	if string.sub(t[1], 1, 1) == ":" then
  	  	t.prefix = string.sub(table.remove(t, 1), 2)
  	end
  	t.name = table.remove(t, 1)
  	local num = tonumber(t.name)
  	if num then
		t.name = numerics[num] or t.name
	end
  	return t
end

function IRC:on_read(line)
	local msg  = ircsplit(line)
	local name = string.upper(msg.name)
	if self[name] and type(self[name]) == 'function' then
		self[name](self, msg)
	else
		run_hook('unknown protocol command', { name = msg.name, args = msg, detail = "prefix: " .. tostring(msg.prefix) })
	end

	self:readline()
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

function IRC:message(target, kind, msg)
	local name = target.name
	if target.type == 'room' then
		if not name:match("^[#&]") then
			name = "#" .. name
		end
		run_hook("public message sent", self, name, kind, msg)
	elseif target.type == 'user' then
		run_hook("private message sent", self, name, kind, msg)
	else
		--screen:debug("Unknown target type: %1", target.type)
	end
	
	if kind == 'do' then
		msg = "\001ACTION "..msg.."\001"
	end
	self:send('PRIVMSG %s :%s', name, msg)
end

-- NICK response
function IRC:ERR_NICKNAMEINUSE(msg)
	self:username(msg[2] .. "_")
	self:send("NICK %s", self:username())
end

-- topic response
function IRC:RPL_TOPIC(msg)
	local room, topic = msg[2], msg[3]
	run_hook('topic', self, topic)
end

function IRC:RPL_MOTDSTART(msg)
	local nick, text = msg[1], msg[2]
	self.motd = { text }
end

function IRC:RPL_MOTD(msg)
	-- [WARNING] unknown protocol command: name=372, args=dylan_,-  , detail=prefix: Arcsecant.aftran.com
	local nick, text = msg[1], msg[2]
	table.insert(self.motd, text)
end

function IRC:RPL_ENDOFMOTD(msg)
	local nick, text = msg[1], msg[2]
	table.insert(self.motd, text)

	run_hook('motd', self, self.motd)
	self.motd = nil
end

-- Until we have real color support, this'll at least get the color codes
-- out of the way.
local function stripcolors(line)--{{{
  -- Bold, underline, reverse/italic, reset
  line = line:gsub("[\002\037\026\017]", "")
  line = line:gsub("\003%d+,%d+", "")
  line = line:gsub("\003%d+", "")
  line = line:gsub("\003", "")
  return line
end--}}}

function IRC:PRIVMSG(msg)
	local user = msg.prefix:match("(.+)!")
	local name, text = msg[1], msg[2]
	local ctcp = string.match(text, "^\001(.+)\001$")
	local kind = 'say'

	if ctcp then
		kind, text = string.match(ctcp, "^([A-Z]+) ?(.-)$")
		--screen:debug("CTCP: %1 (%2)", kind, text)
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
			run_hook('public message', self, name, user, kind, text)
		else
			run_hook('private message', self, user, kind, text)
		end
	end
end

return IRC
