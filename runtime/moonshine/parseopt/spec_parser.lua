local lpeg  = require 'lpeg'
local types = require "moonshine.parseopt.types"
local M     = {}

local P, S, R       = lpeg.P, lpeg.S, lpeg.R
local C, Ct, Cc     = lpeg.C, lpeg.Ct, lpeg.Cc

local letter        = R ("az", "AZ") + S "_-"
local word          = letter ^ 1

local argument     = P "#" * Cc '#'
                   + P '*' * Cc '*'
local option_key   = C( word )
local option_alias = Ct( (P "|" * option_key)^0 )
local option_type  = (P "=" * (word / types)) + Cc(types.boolean)
local option = Ct(option_key * option_alias * option_type)
             + argument

local function option_list_func(options)
	local argc       = 0
	local keys       = {}
	local alias_keys = {}

	for i, option in ipairs(options) do
		if type(option) == 'table' then
			local key, aliases, func = option[1], option[2], option[3]
			keys[ key ] = func
			for i, alias_key in ipairs(aliases) do
				alias_keys[alias_key] = key
			end
		elseif option == '#' and argc >= 0 then
			argc = argc + 1
		elseif option == '*' then
			argc = -1
		end
	end

	return { argc = argc, keys = keys, alias_keys = alias_keys }
end

local option_list = Ct(option * (P "," * option)^0) / option_list_func

function M.parse(text)
	return lpeg.match(option_list, text)
end

return M
