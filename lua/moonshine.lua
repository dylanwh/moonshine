require "bind" 
require "ui"
require "cmd"
require "config"

function boot_hook()
	ui.render()
end

function quit_hook()
	ui.print("Shutdown: %1", "bob")
end


