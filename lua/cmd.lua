-- GLOBALS: cmd, eval().
require "haver"

cmd = {}

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
	Haver:new()
end

function cmd.join(text)
	local rname, sname = text:match("(%w+)@(%w+)")
	if rname and sname then
		local server = servers[sname]
		if server then
			server:join(rname)
		else
			screen:debug("Unknown server: %1", sname)
		end
	else
		screen:debug("Sorry, only /join room@server is supported right now.")
	end
end

function cmd.quit(text)
	quit()
end

function cmd.say(line)
	local target = screen.window.target
	if target then
		target.server:msg(target, line)
	end
end

function cmd.topic(text)
	screen.window.topic:set(text)
end
