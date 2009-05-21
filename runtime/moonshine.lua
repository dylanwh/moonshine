require "moonshine.prelude"
require "moonshine.ui"
require "moonshine.log"

local loop = require "moonshine.loop"

function cmd_go()
	protocol.connect('irc', { hostname = os.getenv('IRC_HOST'), port = tonumber(os.getenv('IRC_PORT')) })

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
