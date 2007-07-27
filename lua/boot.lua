package.path = "lua/?.lua"
require("input")
require("ui")


function on_line(ln)
	ui:print(ln)
	ui:render()
end

