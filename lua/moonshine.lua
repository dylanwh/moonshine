require "bless"
require "strict"
require "bind" 
require "ui"
require "cmd"
require "config"

function on_boot()
	ui:render()
end

function on_shutdown()
	ui:print("Shutdown")
end
