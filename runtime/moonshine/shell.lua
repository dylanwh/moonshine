local M = {}

local screen = require "moonshine.ui.screen"

function M.accept_line(text)
    local view = screen.current_view()
    local conv = view:conversation()
    if conv then
        conv:send(text)
    end
end

return M
