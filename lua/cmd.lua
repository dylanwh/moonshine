-- GLOBALS: cmd, eval().

require "haver"

cmd = {}

function cmd.connect(text)
	server = Haver:new()
end

function cmd.join(text)
	room = text
	server:join(text)
end

function cmd.quit(text)
	quit()
end

function cmd.say(line)
	server:send('IN', room, 'say', line)
end

function cmd.topic(text)
	screen.window.topic:set(text)
end

function eval(line)
	local word, arg = line:match("^/(%w+) ?(.*)")
	if word then
		word = word:lower()
		if cmd[word] then
			return cmd[word](arg)
		end
	elseif cmd.say then
		return cmd.say(line)
	end
end
