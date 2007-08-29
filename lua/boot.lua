require("input")
require("ui")


function on_connect(tag)
	ui:print("Conntected: %1", tag)
	ui:render()
end

function on_read(tag, msg)
	ui:print("%1 %|%2", tag, msg)
	ui:render()
end

function on_error(tag)
	ui:print("%{bob}error%{default}: %1", tag)
	ui:render()
end

function on_boot()
	ui:render()
	net.connect('test', 'localhost', 7575)
end

function on_shutdown()
end

app.boot()
