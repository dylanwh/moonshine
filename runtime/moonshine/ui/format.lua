local term     = require "moonshine.ui.term"
local log      = require "moonshine.log"

local Format = new "moonshine.template"
Format.const = {
    ['|'] = term.INDENT_CODE,
    ['^'] = term.STYLE_RESET_CODE,
}

function Format.env.date(fmt, time)
    return os.date(fmt, time)
end

local COLOR_MAP, STYLE_MAP

local function style_code(style)--{{{
    local style_id = STYLE_MAP:find(style)
    if style_id then
        return term.style_code(style_id)
    else
        return ""
    end
end--}}}

function Format.env.style(name, text)--{{{
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

    M.define_style('topic', 'white', 'blue')
    M.define('topic', '$(style topic)$1')
    M.define("timestamp", "$(date '%H:%M' $1)")
    M.define("chat",    "$(timestamp $1) <$2> $3")
    M.define("public",  "$(chat $0)")
    M.define("private", "$(chat $0)")
    M.define('prompt',  "[$1] ")
end--}}}

function M.define_color(color, r, g, b)--{{{
    assert(type(color) == "string", "color must be string, not " .. type(color))
    assert(type(r) == "number", "red value must be number, not " .. type(r))
    assert(type(g) == "number", "green value must be number, not " .. type(g))
    assert(type(b) == "number", "blue value must be number!, not " .. type(b))
    term.color_init( COLOR_MAP:find_or_assign(color), r, g, b)
end--}}}

function M.define_style(style, fg, bg)--{{{
    assert(type(style) == 'string', "style must be string, not " .. type(style))
    assert(type(fg) == 'string', "fg must be string, not " .. type(fg))
    assert(type(bg) == 'string', "bg must be string, not " .. type(bg))
    local fg_id = assert(COLOR_MAP:find(fg), "unknown color: " .. fg)
    local bg_id = assert(COLOR_MAP:find(bg), "unknown color: " .. bg)
    term.style_init( STYLE_MAP:find_or_assign(style), fg_id, bg_id)
end--}}}


return M
