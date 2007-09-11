
require "strict"

require "bless"
require "bind" 
require "ui"
require "cmd"
require "config"

declare "on_boot"
function on_boot()
	ui:render()
end

declare "on_shutdown"
function on_shutdown()
	ui:print("Shutdown: %1", "bob")
	ui:render()
end


