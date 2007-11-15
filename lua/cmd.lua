-- GLOBALS: cmd, eval().
require "haver"

cmd = {}

local function frob(s)
	local name, tag, pos = s:match("^([^ ])+@(%w+)()")
	if name and tag and pos then
		return name, tag, string.sub(s, pos + 1)
	elseif screen.window.server then
		local name, pos = s:match("^([^ ]+)()")
		return name, screen.window.server.tag, string.sub(s, pos)
	end
end

function eval(text)
	local word, arg = text:match("^/(%w+) ?(.*)")
	if word then
		word = word:lower()
		if cmd[word] then
			return cmd[word](arg)
		end
	elseif cmd.say then
		return cmd.say(text)
	end
end

function cmd.connect(text)
	local server = Haver:new { hostname = "chat.haverdev.org" }
	if not screen.window.server then
		screen.window.server = server
	end
end

function cmd.join(text)
	local room, tag = frob(text)
	if room and tag then
		local server = servers[tag]
		if server then
			server:join(room)
		else
			screen:debug("Unknown server tag: %1", tag)
		end
	else
		screen:debug("Usage: /join room[@tag]")
	end
end

function cmd.query(text)
	local user, tag, msg = frob(text)
	if user and tag then
		local server = servers[tag]
		if server then
			query_hook(server, user)
			if msg and msg:len() > 0 then
				server:msg({ type = 'user', name = user }, 'say', msg)
			end
		else
			screen:debug("Unknown server tag: %1", tag)
		end
	else
		screen:debug("Usage: /query user[@host] [message]")
	end
end

function cmd.quit(text)
	quit()
end

function cmd.me(line)
	local window = screen.window
	local server = window.server
	if server then
		server:msg(window, 'do', line)
	end
end

function cmd.say(line)
	local window = screen.window
	local server = window.server
	if server then
		server:msg(window, 'say', line)
	end
end

function cmd.names(room)
	local window = screen.window
	local server = window.server
	if server then
		server:userlist(window)
	end
end

function cmd.topic(text)
	screen.window.topic:set(text)
end
