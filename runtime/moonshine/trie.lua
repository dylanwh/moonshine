local Object = require "moonshine.object"
local Trie   = require "moonshine.trie"

local function is_terminal(t)
	assert(t)
	return not not t['']
end

local function insert(trie, key, val)
	for ch in key:gmatch(".") do
		if not trie[ch] then
			trie[ch] = {}
		end
		trie = trie[ch]
	end
	trie[''] = { key = key, val = val }
	return trie['']
end

local function lookup(trie, key)
	local buf = ""
	for ch in key:gmatch(".") do
		if not trie[ch] then
			return buf, nil
		end
		trie = trie[ch]
		buf = buf .. ch
	end

	return buf, trie
end

function Trie:__init()
end

function Trie:is_terminal(

return Trie
