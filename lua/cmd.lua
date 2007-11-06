-- GLOBALS: cmd, eval().

cmd = {}

function cmd.quit(text)
	quit()
end

function cmd.connect(text)
	connect("lofn.sinedev.org", 7575)
end

function cmd.join(text)
	local h = ui.window.handle
	if h then
		ui.window.room = text
		ui.window:set_topic("Room: "..text)
		h:write("JOIN\t"..text.."\n")
	end
end

function cmd.say(line)
	local h = ui.window.handle
	local room = ui.window.room
	if h and room then
		h:write("IN\t"..room.."\tsay\t"..line.."\n")
	else
		print(line)
	end
end

function cmd.topic(text)
	ui.topic:set(text)
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
