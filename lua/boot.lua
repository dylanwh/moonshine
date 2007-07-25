-- Eventually, with a saner keybinding system (bd_?)
-- bind() and unbind() can be C primitives.
function bind(spec, f)
	local i = keymap:define(spec)
	keymap.event[i] = f
end

function unbind(spec)
	local i = keymap:undefine(spec)
	keymap.event[i] = nil
end
