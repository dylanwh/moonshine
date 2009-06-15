local numerics = require "moonshine.protocol.irc.numerics"
local Protocol = require "moonshine.protocol.simple"
local log      = require "moonshine.log"
local IRC = Protocol:clone()

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


function IRC:on_connect()
	self:send("NICK %s", self:username())
	self:send('USER %s hostname servername :%s', self:username(), 'Moonshine User')
	self:trigger('connected')
	self:readline()
end

function IRC:write(msg)
	assert(#msg <= 510)
	return Protocol.write(self, msg .. "\r\n")
end

function IRC:send(fmt, ...)
	local ok, str = pcall(string.format, fmt, ...)
	if ok then
		self:write(str)
	else
		error(str, 2)
	end
end

function IRC:on_read(line)
	local msg  = ircsplit(line)
	local name = msg.name:upper()
	if self[name] and type(self[name]) == 'function' then
		self[name](self, msg.prefix, unpack(msg))
	else
		self:trigger('unknown protocol command', { name = msg.name, args = { msg.prefix, unpack(msg) } })
	end

	self:readline()
end

function IRC:join(room)
	assert(room)

	if not room:match("^[#&]") then
		room = "#" .. room
	end
	self:send('JOIN %s', room)
end

function IRC:part(room)
	assert(room)

	if not room:match("^[#&]") then
		room = "#" .. room
	end
	self:send('PART %s', room)
end

function IRC:_message(word, target, type, text)
	assert(word, 'word')
	assert(target, 'target')
	assert(type, 'type')
	assert(text, 'text')
	
	if type == 'action' then
		text = "\001ACTION "..text.."\001"
	end

	self:send('PRIVMSG %s :%s', target, text)
	if word == 'public' then
		-- from room, from user (= me), type, text
		self:trigger('sent public message', target, self:username(), type, text)
	elseif word == 'private' then
		-- from user (= me), to user, type, text
		self:trigger('sent private message', target, type, text)
	end
end

function IRC:public_message(...)
	return self:_message('public', ...)
end

function IRC:private_message(...)
	return self:_message('private', ...)
end

-- NICK response
function IRC:ERR_NICKNAMEINUSE(prefix, _, name)
	self:username(name .. "_")
	self:send("NICK %s", self:username())
end

-- topic response
-- RPL_TOPIC (Cosecant.aftran.com,dylan_,#basement,[dee-doo-doodoodoo, doo-doodeedoo~] It then slobbers on your leg.
function IRC:RPL_TOPIC(prefix, me, room, topic)
	self:trigger('topic', room, topic)
end

function IRC:RPL_MOTDSTART(prefix, nick, text)
	self.motd = { text }
end

function IRC:RPL_MOTD(prefix, nick, text)
	table.insert(self.motd, text)
end

function IRC:RPL_ENDOFMOTD(prefix, nick, text)
	table.insert(self.motd, text)

	self:trigger('motd', self.motd)
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

function IRC:is_channel(target)
	if target:match('^[#&]') then
		return true
	else
		return false
	end
end

function IRC:PRIVMSG(prefix, target, text)
	local user   = prefix:match("(.+)!")
	local ctcp   = text:match("\001(.+)\001")

    text = stripcolors(text)
	if ctcp then
		local cmd, pos = ctcp:match("^(%w+)()")
		local func     = self['CTCP_' .. cmd]
		local body     = ctcp:sub(pos + 1)
		if func then
			func(self, prefix, target, body)
		else
			self:trigger('unknown protocol command', { name = 'CTCP_' .. cmd, args = { prefix, target, body } })
		end
		self:trigger('irc ctcp ' .. cmd:lower(), prefix, target, body)
	else
		if self:is_channel(target) then
			-- (from room, from user, type, text)
			self:trigger('public message', target, user, 'normal', text)
		else
			-- (from user, type, text)
			self:trigger('private message', user, 'normal', text)
		end
	end
end

function IRC:NOTICE(prefix, target, text)
	local user = prefix:match("(.+)!") or prefix

    text = stripcolors(text)
	if self:is_channel(target) then
		self:trigger('public message',  target, user, 'notice', text)
	else
		self:trigger('private message', user, 'notice', text)
	end
end

function IRC:CTCP_ACTION(prefix, target, text)
	local user = prefix:match("(.+)!") or prefix

	if self:is_channel(target) then
		self:trigger('public message', target, user, 'action', text)
	else
		self:trigger('private message', user, 'action', text)
	end
end

function IRC:CTCP_VERSION(prefix, target, text)
	local user   = prefix:match("(.+)!")
	self:send('NOTICE %s :\001VERSION Moonshine %s\001', user, VERSION)
end

function IRC:CTCP_PING(prefix, target, text)
	local user   = prefix:match("(.+)!")
	self:send('NOTICE %s :\001PING %s\001', user, text)
end

function IRC:PING(_, server)
	self:send('PONG %s', server)
end


return IRC
