package.path = "lua/?.lua"
require("input")
require("ui")

function print(...)
	ui:print(arg[1])
	ui:render()
end

function on_line(ln)
	ui:print(ln)
	ui:render()
end

