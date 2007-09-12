keypress_hook = ui.keypress

bind("^[[D", ui.key_left)
bind("^[[C", ui.key_right)
bind("^?",  ui.key_backspace)

bind("^C", quit)
bind("^X", quit)

bind("^M", function ()
	local line = ui.entry:get()
	eval(line)
	ui.entry:clear()
	ui.render()
end)

