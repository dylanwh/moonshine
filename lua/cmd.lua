-- GLOBALS: cmd, eval().

cmd = {}

function cmd.quit(text)
	quit()
end

function cmd.test(text)
	net.connect("lofn.sinedev.org", 7575, function (fd, error)
		if fd then
			ui.print("connected with %1", fd)
			handle = Handle.new(fd, function (handle, event, arg)
				ui.print("event = %1, arg = %2", event, arg)
			end)
			assert(getmetatable(handle))
			handle:write("HAVER\tfoobar\n")
			handle=nil
		end
	end)
end

function cmd.spam(text)
	for i = 1, 10 do
		ui.print(text)
	end
end

function cmd.foo(text)
	for k,v in pairs(ClientRef) do
		ui.print("%1 %|%2", k, v)
	end
end

function cmd.say(line)
	ui.print(line)
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
