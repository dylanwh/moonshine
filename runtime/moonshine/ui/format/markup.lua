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
local format        = require "moonshine.ui.format"
local lpeg          = require "lpeg"
local P, R, S, V    = lpeg.P, lpeg.R,  lpeg.S,  lpeg.V
local C, Ct, Cc, Cs = lpeg.C, lpeg.Ct, lpeg.Cc, lpeg.Cs
local match         = lpeg.match

local letter  = R('AZ','az') + '_'
local digit   = R '09'
local space   = S " \t\r\n"
local name    = letter * (letter + digit)^0

local quote   = function(chr)
    local any = (P ("\\" .. chr) / chr)
              + (P "\\\\" / "\\")
              + (P(1) - chr)
    return chr * Cs( any^0 ) * chr
end
local quoted = quote([[']]) + quote([["]])

local ENTITY_MAP = { amp = '&', lt = '<', gt = '>', quot = [["]], apos = [[']] }

local cdata  = (P(1) - S '&<>')^1
local entity = ('&' * C(name) * ';') / ENTITY_MAP

local function I(str)
    return P(str:upper()) + P(str:lower())
end

local function link(t)
    local href, text = unpack(t)
    return format.apply('link', href, text)
end

local function bold(text)
    return format.apply("bold", text)
end

local markup = P {
    'text',
    text    = Cs( (cdata + entity + V 'markup')^1 ),
    markup  = V 'link' + V 'bold',
    link    = Ct (I '<a' * space^1 * I 'href' * '=' * quoted * space^0 * '>' * V 'text' * I '</a>') / link,
    bold    = I '<b>' * C(V 'text') * I '</b>' / bold,
}

format.define('markup', function(text)
    return match(markup, text)
end)

