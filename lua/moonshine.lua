
require "strict"

require "bless"
require "bind" 
require "ui"
require "cmd"
require "config"


declare "on_client"
function on_client(client, event, msg)
	ui:print("%1%| %2", event, msg)
	ui:render()
end

declare "c"
declare "on_boot"
function on_boot()
	ui:render()
	c = Client.new("localhost", 7575, on_client)
	c:connect()
end

declare "on_shutdown"
function on_shutdown()
	ui:print("Shutdown: %1", "bob")
	ui:render()
end


