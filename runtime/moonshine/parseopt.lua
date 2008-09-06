-- vim: set et:
local lpeg          = require "lpeg"
local parseopt_core = require "moonshine.parseopt.core"
local Object        = require "moonshine.object"
local Parseopt      = Object:clone()

-- {{{ spec parsing stuff
local HINTS = { s = 'string', n = 'number', t = 'table', l = 'list', b = 'boolean', i = 'counter' }

local P, S, R       = lpeg.P, lpeg.S, lpeg.R  
local C, Ct, Cc     = lpeg.C, lpeg.Ct, lpeg.Cc

local letter        = R ("az", "AZ") + S "_-"
local word          = letter ^ 1
local option_name   = C ( word )
local option_hint   = S "sntlbc" / HINTS
local option_rest   = (P "|" * option_name) ^ 0
local option_names  = Ct (option_name * option_rest)
local option        = option_names * ( (P "=" * option_hint) + Cc 'boolean' )

local function parse_spec (s)
    return lpeg.match(option, s)
end
-- }}}

function Parseopt:init(...)--{{{
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
end--}}}

function Parseopt:run(text)--{{{
	local options = {}
	local args    = {}
	for name, value in pairs (self.hints) do
		if value == 'list' or value == 'table' then
			options[name] = {}
		end
	end

	local rest = parseopt_core.run(text, self:callback("on_option", options, args))
	table.insert(args, rest)
	return options, unpack(args)
end--}}}

function Parseopt:on_option(options, args, name, value)--{{{
	local hints = self.hints
	local alias = self.alias
	if name == nil then
		if hints[ #args + 1] then
			table.insert(args, value)
			return parseopt_core.NOARG
		else
			return parseopt_core.STOP
		end
	elseif alias[name] then
		local primary = alias[name]
		local hint    = hints[primary]
		local method  = self["on_option_" .. hint]
		local val = method(self, options, primary, value)
		local ret
		for k, v in pairs(parseopt_core) do
			if v == val then
				ret = k
				break
			end
		end
		return val
	end
end--}}}

function Parseopt:on_option_boolean(options, primary)--{{{
	options[primary] = true
	return parseopt_core.NOARG
end--}}}

function Parseopt:on_option_string(options, primary, value)--{{{
	options[primary] = value
	return parseopt_core.EATARG
end--}}}

function Parseopt:on_option_number(options, primary, value)--{{{
	options[primary] = tonumber(value)
	return parseopt_core.EATARG
end--}}}

function Parseopt:on_option_list(options, primary, value)--{{{
	table.insert(options[primary], value)
	return parseopt_core.EATARG
end--}}}

return Parseopt
