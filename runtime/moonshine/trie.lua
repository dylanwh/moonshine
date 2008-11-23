local M = {}


-- returns (true, key) if the trie has a value associated with key,
-- returns false if key is a valid prefix into the trie.
-- returns nil if the key is not a valid prefix into the trie.
local function find(node, key, i)
	local c = key:sub(i, i)
	if c == '' then
		if node.value ~= nil then
			return true, node.value
		else
			return false
		end
	else
		local child = node[c]
		if not child then
			return nil
		else
			return find(child, key, i+1)
		end
	end
end

function M.find(node, key)
	assert(type(node) == 'table', "trie must be a table")
	assert(type(key) == 'string', "trie 'key' must be a string")

	return find(node, key, 1)
end

-- insert a key=value association into the trie (a table).
-- Note: key must be a non-empty string, and value must not be nil.
function M.insert(node, key, value)
	assert(type(node) == 'table', "trie must be a table")
	assert(type(key) == 'string', "trie 'key' must be a string")
	assert(type(value) ~= 'nil', "cannot insert nil value into trie")

	for c in key:gmatch(".") do
		if not node[c] then
			node[c] = {}
		end
		node = node[c]
	end
	node.value = value or true
end

function M:new()
	local object = { trie = {} }

	function object:find(key) 
		return M.find(self.trie, key) 
	end

	function object:insert(key, value)
		return M.insert(self.trie, key, value)
	end

	return object
end

return M
