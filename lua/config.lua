
bind("^[[D", cmd.left)
bind("^[[C", cmd.right)
bind("^?",  cmd.backspace)
bind("^C", app.shutdown)
bind("^X", app.shutdown)
bind("^M", function ()
	local line = ui:get()
	eval(line)
	ui:clear()
	ui:render()
end)

cmd.quit = app.shutdown
