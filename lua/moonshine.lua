require "bless"
require "bind" 
require "ui"
require "cmd"
require "config"


function client_hook(client, event, msg)
	ui.print("%1%| %2", event, msg)
	if event == 'connect' or event == 'read' then
		client:readline()
	end
	ui.render()
end

function boot_hook()
	ui.render()
	do
		local c = Client.new("localhost", 7575, client_hook)
		c:connect()
	end
	collectgarbage();
end

function shutdown_hook()
	ui.print("Shutdown: %1", "bob")
	ui.render()
end

