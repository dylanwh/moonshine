local M = {}
local color = require "moonshine.ui.color"

function M.init(name, fg, bg)
    term.init_pair(
    color.find(fg)
end

function M.find(name)
    return name_to_id[name]
end

function M.code(name)
    local id = M.find(name)
    if id then
        return term.color_code(id)
    else
        return ""
    end
end

return M
