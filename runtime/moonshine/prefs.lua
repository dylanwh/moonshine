local function include(file, env)
	local f    = loadfile(file)
	local vars = {}
	local mt   = { __index = env or _G }

	setmetatable(vars, mt)
	setfenv(f, vars)
	local ok, err = pcall(f)
	setmetatable(vars, nil)

	if err then
		vars.ERROR = err
	end

	return vars
end

local ok, prefs = pcall(include, os.getenv("HOME") .. "/.moonshine/prefs")

if not ok then prefs = {} end

return prefs
