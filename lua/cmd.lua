-- GLOBALS: cmd, eval(). 

declare "cmd"
declare "eval"

cmd = {}

function cmd.QUIT(text)
	shutdown()
end
function cmd.unknown(word, arg)
	ui:print("word: %1, arg = %2", word, arg)
end

function cmd.SPAM(text)
	for i = 1, 200 do
		ui:print(text)
	end
end

function cmd.TOPIC(text)
	ui:set_topic(text)
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
