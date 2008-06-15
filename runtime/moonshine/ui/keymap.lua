local object = require "moonshine.object"
local term   = require "moonshine.ui.term"

local keymap = object:clone {
	__type = 'keymap',
	clone = nil,
	keys = {},
	key = nil
}

assert(keymap.keys)

local function index(s, i) return s:sub(i+1, i+1) end

function keymap:bind(spec, f)
	if spec ~= '' then
		local k = self.keys
		local spec = term.make_keyspec(spec)
		local last = spec:len() - 1
		for i = 0, last, 1 do
			local x = index(spec, i)
			if i == last then
				k[x] = f
			else
				if k[x] == nil then
					k[x] = {}
				end
				k = k[x]
			end
		end
	else
		self.keys[""] = f
	end
end

function keymap:process(k)
	local key = self.key
	local keys = self.keys

	if type(key) == 'table' then
		key = key[k]
	else
		key = keys[k]
		if not key and keys[""] then
			keys[""](k)
		end
	end

	if type(key) == 'function' then
		key()
		key = nil
	end

	self.key = key
end

return keymap
