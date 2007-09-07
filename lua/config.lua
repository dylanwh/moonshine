bind("^[[D", ui.left)
bind("^[[C", ui.right)
bind("^?",  ui.backspace)

bind("^C", app.shutdown)
bind("^X", app.shutdown)

bind("^M", function ()
	local line = ui:get()
	eval(line)
	ui:clear()
	ui:render()
end)
