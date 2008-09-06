local Object = require "moonshine.object"
local term   = require "moonshine.ui.term"

local KeyMap = Object:clone()

local function index(s, i) return s:sub(i+1, i+1) end

function KeyMap:init(...)--{{{
	self.keys = {}
	self.key = nil
	Object.init(self, ...)
	return self
end--}}}

function KeyMap:bind(spec, f)--{{{
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
end--}}}

function KeyMap:process(k)--{{{
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
end--}}}

return KeyMap
