-- imports
local require = require
local ipairs  = ipairs
local table   = table
local signals = {}

module "moonshine.signal"

function emit(name, ...)
	if signals[name] then
		for i, f in ipairs(signals[name]) do
			if f(...) == false then
				return false
			end
		end
	end
	return true
end

function add(name, f)
	if not signals[name] then
		signals[name] = {}
	end

	table.insert(signals[name], f)
end

function add_first(name, f)
	if not signals[name] then
		signals[name] = {}
	end

	table.insert(signals[name], 1, f)
end

function clear(name)
	signals[name] = nil
end
