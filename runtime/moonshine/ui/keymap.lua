local make_keyspec  = require("moonshine.ui.term").make_keyspec
local Object = require "moonshine.object"
local Trie   = require "moonshine.trie"

local KeyMap = Object:new()

getmetatable(KeyMap).__init__ = 

function KeyMap:bind(spec, func)

function M.bind(spec_, name, ...)--{{{
	assert(spec_, "keyspec is required")
	assert(name, "event name is required")

	local spec  = term.make_keyspec(spec_)
	local event = { name = name, ... }

	keymap:insert(spec, event)
end--}}}

local function process(key)--{{{
	keybuf = keybuf .. key
	
	local found, event = keymap:find(keybuf)
	if found then
		event.emit(event.name, unpack(event))
		keybuf = ""
	elseif found == nil then
		event.emit("keypress", key)
		keybuf = ""
	end
end--}}}

event.add("input", process)
return M
