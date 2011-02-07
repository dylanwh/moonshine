local core        = require "moonshine.parseopt.core"
local types       = require "moonshine.parseopt.types"
local spec_parser = require "moonshine.parseopt.spec_parser"

local NOARG, EATARG, STOP = core.NOARG, core.EATARG, core.STOP

function prefix_match(str, prefix)
	return #prefix <= #str and str:sub(1, #prefix) == prefix
end

local function find(keys, alias_keys, name, seen)
	seen = seen or {}
	seen[name] = true

	if alias_keys[name] and not seen[alias_keys[name]] then
		return find(keys, alias_keys, alias_keys[name], seen)
	elseif keys[name] then
		return name, keys[name]
	elseif #name > 1 then
		-- for more than one letter options, we attempt to match
		-- by prefix.
		local matches = {}
		for k, v in pairs(alias_keys) do
			if prefix_match(k, name) then
				table.insert(matches, v)
			end
		end

		for k, v in pairs(keys) do
			if prefix_match(k, name) then
				table.insert(matches, k)
			end
		end

		if #matches == 1 then
			return find(keys, alias_keys, matches[1], seen)
		else
			return nil
		end
	end
end

-- --options and arguments

local function parse(spec, text)
	assert(type(spec) == 'table')
	local argc       = spec.argc
	local keys       = spec.keys
	local alias_keys = spec.alias_keys

	local vars = {}
	local args = {}
	local unknown_option
	local function callback(name, value)
		if name == nil then
			if argc < 0 or #args < argc then
				table.insert(args, value)
				return EATARG
			else
				return STOP
			end
		else
			local key, f = find(keys, alias_keys, name)
			if key then
				local flag
				flag, vars[key] = f(value, vars[key])
				return flag
			else
				print("unknown option: ", name)
				return STOP
			end
		end
	end

	local rest = core.parse(text, callback)
	if argc >= 0 then
		table.insert(args, rest)
	end
	return vars, unpack(args)
end

local function build_parser(spec)
	if type(spec) == 'string' then
		spec = spec_parser.parse(spec)
	end

	return function(text)
		return parse(spec, text)
	end
end

return {
	parse_spec   = spec_parser.parse,
	types        = types,
	parse        = parse,
	build_parser = build_parser,
}
