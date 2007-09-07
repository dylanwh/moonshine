require("bind")
require("cmd")
require("ui")


function on_net_event(tag, event, buf)
	ui:print("%1 %| %2 %3", net.hostname(tag), event, buf)
	ui:render()
	if event == 'connect' or event == 'read' then
		net.readline(tag)
	end
end

function on_boot()
	require("config")
	net.create("test", "localhost", 7575, on_net_event)
	net.connect("test")
	ui:render()
	bind("^M", function ()
		local line = ui:get()
		ui:print(">> %| %1, %2",
		pcall(function ()
			net.write('test', line .. "\n") 
		end))

		ui:clear()
		ui:render()
		pcall(net.destroy, 'test')
	end)
end

function on_shutdown()
end

app.boot()
