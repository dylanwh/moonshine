-- API
-- keymap.bind('^[1', 'window_goto', 1)
--
-- keymap.process('^[')
-- keymap.process('1')
--
-- Results of above two calls to process():
-- kb_window_goto(1)
local M = {}

local Tree         = require "moonshine.tree"

local mapping = Tree:new()
local keybuf  = ""

local function keyspec(spec)
	local bit = require "bit"

	return spec:gsub("%^(.)", function(c)
		return string.char(bit.bxor(string.byte(string.upper(c)), 64))
	end)
end

function M.bind(spec_, name, ...)--{{{
	local spec = keyspec(spec_)
	local extra = { ... }
	local cb = function()
		M.invoke(name, unpack(extra))
	end

	mapping:insert(spec, cb)

	local found_value = mapping:find(spec)
end--}}}

function M.process(key)--{{{
	keybuf = keybuf .. key

	local found_key, func, index, dirn = mapping:find_near(keybuf)

	if found_key == nil then
		-- No keys defined??
		return
	end

	if dirn == 0 then
		func()
		keybuf = ""
		return
	elseif dirn > 0 then
		index = index + 1
		found_key, func = mapping:lookup_index(index)
	end

	if found_key == nil then
		-- should only ever happen if there are no keys registered at all
		found_key = ""
	end

	if string.sub(found_key, 1, string.len(keybuf)) ~= keybuf then
		-- not a prefix of anything
		emit("keypress", key)
		keybuf = ""
	else
		-- prefix of something, keep going
	end
end--}}}

function M.invoke(name, ...)--{{{
	local func = _G["kb_" .. name]
	if func then 
		func(...)
	end
end--}}}

return M
