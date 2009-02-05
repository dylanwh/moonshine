local event = require "moonshine.event"
local term   = require "moonshine.ui.term"
local trie   = require "moonshine.trie"

local keymap = trie.new()
local keybuf = ""
local M      = {}

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
