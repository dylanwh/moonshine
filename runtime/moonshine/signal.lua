local signals = {}
local M       = {}

local function _emit(name, ...)
    local list = signals[name]
    if list then
        for _, f in ipairs(list) do
            f(...)
        end
    end
end

function M.emit(name, ...)
	if signals[name] then
	    _emit("before " .. name, ...)
    	_emit(name, ...)
    	_emit("after " .. name, ...)
    	return true
    else
    	return false
    end
end

function M.add(name, func)
    signals[name] = signals[name] or {}
    table.insert(signals[name], func)
end

function M.bind(from, to)
	M.add(from, function(...)
		M.emit(to, ...)
	end)
end

return M
