local parse_spec = require "moonshine.pirate.parse_spec"
local parseopt   = require "moonshine.parseopt"
local Object     = require "moonshine.object"
local Pirate     = Object:clone()

function Pirate:init(...)
	local hints = {}
	local alias = {}

	for i, spec in ipairs { ... } do
		if type(spec) == 'number' then
			hints[spec] = true
		else
			local names, hint = parse_spec(spec)
			local primary     = table.remove(names, 1)
			hints[primary]    = hint
			alias[primary]    = primary
			for i, name in ipairs (names) do
				alias[name] = primary
			end
		end
	end
	self.hints = hints
	self.alias = alias
end

function Pirate:parse(text)
	local options = {}
	local args    = {}
	for name, value in pairs (self.hints) do
		if value == 'list' or value == 'table' then
			options[name] = {}
		end
	end

	local rest = parseopt.run(text, self:callback("on_option", options, args))
	table.insert(args, rest)
	return options, unpack(args)
end

function Pirate:on_option(options, args, name, value)
	local hints = self.hints
	local alias = self.alias
	if name == nil then
		if hints[ #args + 1] then
			table.insert(args, value)
			return parseopt.NOARG
		else
			return parseopt.STOP
		end
	elseif alias[name] then
		local primary = alias[name]
		local hint    = hints[primary]
		local method  = self["on_option_" .. hint]
		local val = method(self, options, primary, value)
		local ret
		for k, v in pairs(parseopt) do
			if v == val then
				ret = k
				break
			end
		end
		return val
	end
end

function Pirate:on_option_boolean(options, primary)
	options[primary] = true
	return parseopt.NOARG
end

function Pirate:on_option_string(options, primary, value)
	options[primary] = value
	return parseopt.EATARG
end

function Pirate:on_option_number(options, primary, value)
	options[primary] = tonumber(value)
	return parseopt.EATARG
end

function Pirate:on_option_list(options, primary, value)
	table.insert(options[primary], value)
	return parseopt.EATARG
end


return Pirate
