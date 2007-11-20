-- GLOBALS: cmd, eval().
require "protocol.irc"
require "protocol.haver"

cmd = {}

local function frob(s)
	local name, tag, pos = s:match("^([^ ])+@(%w+)()")
	if name and tag and pos then
		return name, tag, string.sub(s, pos + 1)
	elseif screen.window.protocol then
		local name, pos = s:match("^([^ ]+)()")
		return name, screen.window.protocol.tag, string.sub(s, pos)
	end
end

local function _eval(text)
	local word, arg = text:match("^/([%w_-]+) ?(.*)")
	if word then
		word = word:lower()
		if cmd[word] then
			return cmd[word](arg)
		end
	elseif cmd.say then
		return cmd.say(text)
	end
end



function eval(text)
	ok, err = pcall(_eval, text)
	if not ok then
		screen:debug("Error: %1", err)
	end
end


function cmd.open(text)
	local t = shell_parse(text)
	for i, x in ipairs(t) do
		screen:debug("%1 = %2", i, x)
	end
end

function cmd.connect(text)
	local protocol = Haver:clone()
	protocol:connect("chat.haverdev.org", 7575)
	if not screen.window.protocol then
		screen.window.protocol = protocol
	end
end

function cmd.ircconnect(text)
	local protocol = IRC:clone()
	protocol:connect("irc.example.com", 6666)
	if not screen.window.protocol then
		screen.window.protocol = protocol
	end
end

function cmd.join(text)
	local room, tag = frob(text)
	if room and tag then
		local protocol = protocols[tag]
		if protocol then
			protocol:join(room)
		else
			screen:debug("Unknown protocol tag: %1", tag)
		end
	else
		screen:debug("Usage: /join room[@tag]")
	end
end

function cmd.query(text)
	local user, tag, msg = frob(text)
	if user and tag then
		local protocol = protocols[tag]
		if protocol then
			query_hook(protocol, user)
			if msg and msg:len() > 0 then
				protocol:msg({ type = 'user', name = user }, 'say', msg)
			end
		else
			screen:debug("Unknown protocol tag: %1", tag)
		end
	else
		screen:debug("Usage: /query user[@host] [message]")
	end
end

function cmd.quit(text) quit() end

function cmd.me(line)
	local window = screen.window
	local protocol = window.protocol
	if protocol then
		protocol:msg(window, 'do', line)
	end
end

function cmd.say(line)
	local window = screen.window
	local protocol = window.protocol
	if protocol then
		protocol:msg(window, 'say', line)
	end
end

function cmd.names(room)
	local window = screen.window
	local protocol = window.protocol
	if protocol then
		protocol:userlist(window)
	end
end

function cmd.protocols(text)
	screen:debug("Protocols: ")
	for k, v in pairs(protocols) do
		screen:debug("    %1", k)
	end
end

function cmd.topic(text)
	screen.window:set_topic(text)
end

function cmd.lastlog(text)
	local max = 100 -- TODO: user preference
	if text == '-clear' then
		screen.window.buffer:clear_group_id(1)
	else
		screen.window.buffer:set_group_id(1)
		screen.window:print("%{notice}Lastlog for %1:", text)
		local n, err = screen.window.buffer:reprint(function (line)
			return string.match(line, text)
		end, 0, max)
		if not n then
			screen.window.buffer:clear_lines(1)
			screen.window:print("%{notice}Lastlog for %1 failed: %2", text, err )
		elseif n > max then
			screen.window.buffer:clear_lines(1)
			screen.window:print("%{notice}Lastlog for %1 would print %2 lines.", text, n)
		else
			screen.window:print("%{notice}End of lastlog.")
		end
		screen.window.buffer:set_group_id(0)
	end
end

