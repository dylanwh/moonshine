-- GLOBALS: cmd, eval(). 

cmd = {}

function cmd.QUIT(text)
	app.shutdown()
end

function eval(line)
	word, arg = line:match("/(%w+) ?(.*)")
	if word then
		WORD = string.upper(word)
		if cmd[WORD] then
			return cmd[WORD](arg)
		elseif cmd.unknown then
			return cmd.unknown(word, arg)
		end
	elseif cmd.default then
		return cmd.default(line)
	end
end
