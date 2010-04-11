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

local term = require "moonshine.ui.term"
local log  = require "moonshine.log"

local Format = new "moonshine.template"
Format.const = {
    ['|'] = term.INDENT_CODE,
    ['^'] = term.STYLE_RESET_CODE,
}

local COLOR_MAP, STYLE_MAP

local function style_code(style)--{{{
    local style_id = STYLE_MAP:find(style)
    if style_id then
        return term.style_code(style_id)
    else
        return ""
    end
end--}}}

local M = {
    define     = Format:callback "define",
    apply      = Format:callback "apply",
    eval       = Format:callback "eval",
    style_code = style_code,
}

function M.init()--{{{
    local colors    = term.colors()
    local styles    = term.styles()
    assert(colors > 0 and styles > 0, "moonshine.ui.term not initialized?")

    COLOR_MAP = new("moonshine.map", colors)
    STYLE_MAP = new("moonshine.map", styles)

    COLOR_MAP:assign('default',  0)
    COLOR_MAP:assign('black',    1)
    COLOR_MAP:assign('red',      2)
    COLOR_MAP:assign('green',    3)
    COLOR_MAP:assign('yellow',   4)
    COLOR_MAP:assign('blue',     5)
    COLOR_MAP:assign('magenta',  6)
    COLOR_MAP:assign('cyan',     7)
    COLOR_MAP:assign('white',    8)
    COLOR_MAP:assign('black2',   9)
    COLOR_MAP:assign('red2',     10)
    COLOR_MAP:assign('green2',   11)
    COLOR_MAP:assign('yellow2',  12)
    COLOR_MAP:assign('blue2',    13)
    COLOR_MAP:assign('magenta2', 14)
    COLOR_MAP:assign('cyan2',    15)
    COLOR_MAP:assign('white2',   16)

    STYLE_MAP:assign('default',  0)
    term.style_init(0, 0, 0)

    M.define('style', function (name, text)
        local code = style_code(name)
        if code == "" then
            return text or ""
        else
            if text then
                return code .. text .. term.STYLE_RESET_CODE
            else
                return code
            end
        end
    end)

        local os = os
    M.define('date', function (...) return os.date(...) end)
    M.define('now',  function (...) return os.time()    end)

    -- this is split off into a seperate file for sanity's sake.
    require "moonshine.ui.format.markup"
end--}}}

function M.define_color(color, r, g, b)--{{{
    assert(type(color) == "string", "color must be string, not " .. type(color))
    assert(type(r) == "number", "red value must be number, not " .. type(r))
    assert(type(g) == "number", "green value must be number, not " .. type(g))
    assert(type(b) == "number", "blue value must be number!, not " .. type(b))
    term.color_init( COLOR_MAP:find_or_assign(color), r, g, b)
end--}}}

-- FIXME: don't allocate duplicate (fg, bg) styles, instead make an alias.
function M.define_style(style, fg, bg)--{{{
    assert(type(style) == 'string', "style must be string, not " .. type(style))
    assert(type(fg) == 'string', "fg must be string, not " .. type(fg))
    assert(type(bg) == 'string', "bg must be string, not " .. type(bg))
    local fg_id = assert(COLOR_MAP:find(fg), "unknown color: " .. fg)
    local bg_id = assert(COLOR_MAP:find(bg), "unknown color: " .. bg)
    term.style_init( STYLE_MAP:find_or_assign(style), fg_id, bg_id)
end--}}}

return M
