
local atexit_funcs = {}

function atexit(f)
	table.insert(atexit_funcs, f)
end

function exit_hook()
	for _, f in ipairs(atexit) do
		f()
	end
end

function hook (name)
	return function ()
		local f = _G[name .. "_hook"]
		if f then
			return f()
		else
			error("hook " .. name .. " is not defined")
		end
	end
end
