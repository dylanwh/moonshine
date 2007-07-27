package.path = "lua/?.lua"
require("input")
require("ui")

function print(...)
	ui:print(arg[1])
	ui:render()
end

function on_msg(ln)
	ui:print(ln)
	ui:render()
end


ui:print("<%1> %|%2", "dylan", "bunnies ................................................................................................................................................");
ui:render()
app.refresh()
