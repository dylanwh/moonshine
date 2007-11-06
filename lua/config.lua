keypress_hook = ui.keypress

--bind("^[[A", ui.key_up)
--bind("^[[B", ui.key_down)

bind("^[[C", ui.key_right)
bind("^[[D", ui.key_left)
bind("^[[5~", ui.scroll_up);
bind("^[[6~", ui.scroll_down);

bind("^?",  ui.key_backspace)

bind("^C", quit)
bind("^X", quit)

bind("^M", function ()
	local line = ui.entry:get()
	eval(line)
	ui.entry:clear()
	ui.render()
end)

for i = 1, 9 do
	bind("^[" .. i, function ()
		ui.view(i)
		ui.render()
	end)
end


