-- vim: set et:
local HINTS = { s = 'string', n = 'number', t = 'table', l = 'list', b = 'boolean', i = 'counter' }

local lpeg          = require "lpeg"

local P, S, R       = lpeg.P, lpeg.S, lpeg.R  
local C, Ct, Cc     = lpeg.C, lpeg.Ct, lpeg.Cc

local letter        = R ("az", "AZ")
local word          = letter ^ 1
local option_name   = C ( word )
local option_hint   = S "sntlbc" / HINTS
local option_rest   = (P "|" * option_name) ^ 0
local option_names  = Ct (option_name * option_rest)
local option        = option_names * ( (P "=" * option_hint) + Cc 'boolean' )
local index         = R "09" / tonumber
local argument      = option + index

require "dumper"

local function parse_spec (s)
    return lpeg.match(argument, s)
end

return parse_spec
