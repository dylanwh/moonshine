-- GLOBALS: cmd, eval().

cmd = {}

function cmd.quit(text)
	quit()
end

function cmd.say(line)
	screen:print("<you> %|%1", line)
end

function cmd.topic(text)
	screen.window.topic:set(text)
end

function eval(line)
	local word, arg = line:match("^/(%w+) ?(.*)")
	if word then
		word = string.lower(word)
		if cmd[word] then
			return cmd[word](arg)
		end
	elseif cmd.say then
		return cmd.say(line)
	end
end
