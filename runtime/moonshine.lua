require "moonshine.prelude"
require "moonshine.ui"

local loop = require "moonshine.loop"

function cmd_go()
	local IRC = require "moonshine.irc"
	irc = IRC:new()
	irc:connect('irc.example.com', 6666)
end

function cmd_join()
	irc:join('#lobby')
end

function cmd_say(text)
	irc:message( { type = 'room', name = 'lobby' }, 'say', text)
end

emit "startup"
loop.run()
emit "shutdown"
