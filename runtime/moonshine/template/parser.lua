--[[ vim: set ft=lua sw=4 ts=4 expandtab:
-   Moonshine - a Lua-based chat client
-
-   Copyright (C) 2010 Dylan William Hardison
-
-   This file is part of Moonshine.
-
-   Moonshine is free software: you can redistribute it and/or modify
-   it under the terms of the GNU General Public License as published by
-   the Free Software Foundation, either version 3 of the License, or
-   (at your option) any later version.
-
-   Moonshine is distributed in the hope that it will be useful,
-   but WITHOUT ANY WARRANTY; without even the implied warranty of
-   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-   GNU General Public License for more details.
-
-   You should have received a copy of the GNU General Public License
-   along with Moonshine.  If not, see <http://www.gnu.org/licenses/>.
]]

--[[
    $foo           is a macro call
    $(foo)         is a macro call
    $(foo bar baz) is a macro call with arguments.

    Arguments can be:
        * macro calls
        * sugar (described below)
        * quoted strings ("foo" or 'bar')
        * bare words (foo, foo123, foo_123, foo_bar, _)
        * numbers (5, 8.4, .3)

    Sugar:
        * $1       is short for ${param 1}
        * $$       is a literal '$'
        * $|       is short for ${const '|'}
        * $<TEXT>  is special, it causes TEXT to be evaluated as if it was outside of any macro call.

    Literals: Anything that does not begin with a '$' is a literal.
]]
-- {{{ lpeg imports
local lpeg          = require "lpeg"
local P, R, S, V    = lpeg.P, lpeg.R,  lpeg.S,  lpeg.V
local C, Ct, Cc, Cs = lpeg.C, lpeg.Ct, lpeg.Cc, lpeg.Cs
local match         = lpeg.match
-- }}}

-- {{{ grammar
local letter  = R('AZ','az') + '_'
local digit   = R '09'
local space   = S " \t\r\n"
local sp0     = space^0
local sp1     = space^1

local name    = C( letter * (letter + digit)^0 )
local integer = C( digit^1 ) / tonumber
local number  = C( (P('-')^-1) * (digit^1 * '.' * digit^1 + digit^1 + '.' * digit^1) ) / tonumber
local punct   = C( S [=[`~!@#$^&**<>[]-=+|/\?,.]=] )
local quote   = function(chr)
    local any = (P ("\\" .. chr) / chr)
              + (P "\\\\" / "\\")
              + (P(1) - chr)
    return chr * Cs( any^0 ) * chr
end
local squote = quote([[']])
local dquote = quote([["]])

function label(n, p)
    return Ct(Cc(n) * p)
end

-- special opcodes that cannot be generated in templates:
local template = P {
    'root',
    root    = Ct( Cc 'ROOT' * ((V 'literal' + V 'escape')^1)^-1),
    literal = C((P(1) - '$')^1),
    splice  = '<' * sp0 * C(((1 - S '<>') + V 'splice')^0) * sp0 * '>',
    escape  = '$'  * Ct(Cc "APPLY" * name)
            + '$'  * Ct(Cc "APPLY" * Cc "param" * integer)
            + '$(' * sp0 * ')'
            + '$(' * sp0 * Ct(Cc "APPLY" * name * sp0 * V 'tokens') * sp0 * ')'
            + '$(' * sp0 * Ct(Cc "APPLY" * V 'tokens') * sp0 * ')'
            + '$'  * Ct(Cc "EVAL" * V 'splice')
            + '$$' * Cc '$'
            + '$'  * Ct(Cc "APPLY" * Cc "const" * punct),
    tokens  = V 'token' * (sp1 * V 'tokens')^-1,
    token   = name + number + squote + dquote + V 'escape',
}
-- }}}

-- parse a string into a basic ast.
local function parse(text) return match(template, text) end

local TYPE = {}
local OP   = {}

local function compile(op)
    local f = assert(TYPE[ type(op) ], "unknown opcode type: " .. type(op))
    return f(op)
end

local function compile_ops(ops)
    local result = {}
    for i, op in ipairs(ops) do
        result[i] = compile(op)
    end
    return result
end

function TYPE.string(s) return string.format('%q', s) end
function TYPE.number(n) return n end
function TYPE.boolean(b) return tostring(b) end

function TYPE.table(op)
    local name = table.remove(op, 1)
    local f = OP[name] or function() return "'no op named "..name.."'" end
    return f(op)
end

function OP.ROOT(args)
    return string.format("local P = { ... }\nreturn %s", table.concat(compile_ops(args), " .. "))
end

function OP.EVAL(args)
    local code = args[1]
    local op = parse(code)
    op[1] = 'APPLY'
    table.insert(op, 2, "concat")
    table.insert(op, 3, "")
    return compile(op)
end

local keywords = {
    'if', 'elseif', 'then', 'while', 'for', 'do', 'until', 'end',
    'local', 'function', 'return', 'break', 'repeat',
    'true', 'false', 'nil', 'not', 'and', 'or',
    'param', 'P'
}
local is_keyword = {}
for i, k in ipairs(keywords) do is_keyword[k] = true end

local function is_param_zero(args)
    return (
        #args == 1
        and type(args[1]) == 'table'
        and args[1][1] == 'APPLY'
        and args[1][2] == 'param'
        and args[1][3] == 0 
    )
end

function OP.APPLY(args)
    local name  = table.remove(args, 1)
    local args_code
    if is_param_zero(args) then
        args_code = '...'
    else
        args_code = table.concat(compile_ops(args), ", ")
    end

    if name == 'param' then
        if args[1] == 0 then
            return 'concat_0(P)'
        else
            return string.format("(P[%s] or '?')", compile(args[1]))
        end
    elseif type(name) == 'table' then
        return string.format('apply(%s, %s)', compile(name), args_code)
    elseif type(name) == 'number' or is_keyword[name] or not string.match(name, "^[%w_]+$") then
        return string.format('apply(%q, %s)', tostring(name), args_code)
    else
        return string.format('%s(%s)', name, args_code)
    end
end

local M = {}

function M.read(str)
    return compile(parse(str))
end

function M.json(str)
    require 'json'
    return json.encode(parse(str))
end

--[[
print(M.read("$(debug $<this is the debug text. Level: $1, Power: $2>)"))
print(M.read("$(debug $(concat 'this is the debug text. Level: ' $1 ', Power: ' $2))"))
print(M.read("$(local $('bat man' 'this is the debug text. Level: ' $1 ', Power: ' $2))"))
print(M.read("$( $foo foo bar)"))
print(M.read('$( $<$(prefix)_$1> foo bar)'))
]]


return M
