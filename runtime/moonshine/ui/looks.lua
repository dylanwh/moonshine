local M = {}

local COLORS, STYLES
local STYLE_MAP, COLOR_MAP

local term = require "moonshine.ui.term"
local log  = require "moonshine.log"

function M.init()
    COLORS = term.colors()
    STYLES = term.color_pairs()

    COLOR_MAP = new ("moonshine.map", COLORS)
    STYLE_MAP = new ("moonshine.map", STYLES)

    COLOR_MAP:assign('black',   0)
    COLOR_MAP:assign('red',     1)
    COLOR_MAP:assign('green',   2)
    COLOR_MAP:assign('yellow',  3)
    COLOR_MAP:assign('blue',    4)
    COLOR_MAP:assign('magenta', 5)
    COLOR_MAP:assign('cyan',    6)
    COLOR_MAP:assign('white',   7)

    if COLORS > 8 then
        COLOR_MAP:assign('color8',  8)
        COLOR_MAP:assign('color9',  9)
        COLOR_MAP:assign('color10', 10)
        COLOR_MAP:assign('color11', 11)
        COLOR_MAP:assign('color12', 12)
        COLOR_MAP:assign('color13', 13)
        COLOR_MAP:assign('color14', 14)
        COLOR_MAP:assign('color15', 15)
    end

    STYLE_MAP:assign('default', 0)
end

function M.define_color(color, r, g, b)
    assert(type(color) == "string", "color must be string, not " .. type(color))
    assert(type(r) == "number", "red value must be number, not " .. type(r))
    assert(type(g) == "number", "green value must be number, not " .. type(g))
    assert(type(b) == "number", "blue value must be number!, not " .. type(b))
    term.init_color( COLOR_MAP:find_or_assign(color), r, g, b)
end

function M.define_style(style, fg, bg)
    assert(type(style) == 'string', "style must be string, not " .. type(style))
    assert(type(fg) == 'string', "fg must be string, not " .. type(fg))
    assert(type(bg) == 'string', "bg must be string, not " .. type(bg))
    local fg_id = assert(COLOR_MAP:find(fg), "unknown color: " .. fg)
    local bg_id = assert(COLOR_MAP:find(bg), "unknown color: " .. bg)
    log.debug(string.format("init_pair(%d, %d, %d)", STYLE_MAP:find_or_assign(style), fg_id, bg_id))
    term.init_pair( STYLE_MAP:find_or_assign(style), fg_id, bg_id)
end

function M.style_code(style)
    local style_id = STYLE_MAP:find(style)
    if style_id then
        return term.color_code(style_id)
    else
        return ""
    end
end

return M
