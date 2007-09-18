-- GLOBALS: cmd, eval().

cmd = {}

function cmd.QUIT(text)
	quit()
end

function cmd.SPAM(text)
	for i = 1, 10 do
		ui.print(text)
	end
end

function cmd.FOO(text)
	for k,v in pairs(ClientRef) do
		ui.print("%1 %|%2", k, v)
	end
end

function cmd.unknown(word, arg)
	ui.print("word: %1, arg = %2", word, arg)
end

function cmd.default(line)
	ui:print(line)
end

function cmd.TOPIC(text)
	ui.topic:set(text)
end

function eval(line)
	local word, arg = line:match("^/(%w+) ?(.*)")
	if word then
		local WORD = string.upper(word)
		if cmd[WORD] then
			return cmd[WORD](arg)
		elseif cmd.unknown then
			return cmd.unknown(word, arg)
		end
	elseif cmd.default then
		return cmd.default(line)
	end
end
