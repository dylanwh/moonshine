local term     = require "moonshine.ui.term"
local Map      = require "moonshine.map"
local Template = require "moonshine.template"

local COLORS    = term.colors()
local STYLES    = term.styles()
assert(COLORS > 0 and STYLES > 0, "moonshine.ui.term not initialized?")
local COLOR_MAP = Map:new(COLORS)
local STYLE_MAP = Map:new(STYLES)

COLOR_MAP:assign('black',   0)
COLOR_MAP:assign('red',     1)
COLOR_MAP:assign('green',   2)
COLOR_MAP:assign('yellow',  3)
COLOR_MAP:assign('blue',    4)
COLOR_MAP:assign('magenta', 5)
COLOR_MAP:assign('cyan',    6)
COLOR_MAP:assign('white',   7)
COLOR_MAP:assign('black2',   8)
COLOR_MAP:assign('red2',     9)
COLOR_MAP:assign('green2',   10)
COLOR_MAP:assign('yellow2',  11)
COLOR_MAP:assign('blue2',    12)
COLOR_MAP:assign('magenta2', 13)
COLOR_MAP:assign('cyan2',    14)
COLOR_MAP:assign('white2',   15)

STYLE_MAP:assign('default', 0)

local Format = Template:new()

-- Format is a singleton.
function Format:new()
    return Format
end

Format.const = {
    ['|'] = term.INDENT_CODE,
    ['^'] = term.STYLE_RESET_CODE,
}

function Format:define_color(color, r, g, b)
    assert(type(color) == "string", "color must be string, not " .. type(color))
    assert(type(r) == "number", "red value must be number, not " .. type(r))
    assert(type(g) == "number", "green value must be number, not " .. type(g))
    assert(type(b) == "number", "blue value must be number!, not " .. type(b))
    term.color_init( COLOR_MAP:find_or_assign(color), r, g, b)
end

function Format:define_style(style, fg, bg)
    assert(type(style) == 'string', "style must be string, not " .. type(style))
    assert(type(fg) == 'string', "fg must be string, not " .. type(fg))
    assert(type(bg) == 'string', "bg must be string, not " .. type(bg))
    local fg_id = assert(COLOR_MAP:find(fg), "unknown color: " .. fg)
    local bg_id = assert(COLOR_MAP:find(bg), "unknown color: " .. bg)
    term.style_init( STYLE_MAP:find_or_assign(style), fg_id, bg_id)
end

function Format:style_code(style)
    local style_id = STYLE_MAP:find(style)
    if style_id then
        return term.style_code(style_id)
    else
        return ""
    end
end

function Format.env.style(name, text)
    local code = Format:style_code(name)
    if code == "" then
        return text or ""
    else
        if text then
            return code .. text .. term.STYLE_RESET_CODE
        else
            return code
        end
    end
end


return Format
