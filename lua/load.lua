require("bless")
require("strict")
require("bind")
require("ui")
require("cmd")

function on_boot()
	require("config")
	ui:render()
end

function on_shutdown()
	ui:print("Shutdown")
end
