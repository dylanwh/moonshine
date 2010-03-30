local M = {}

local screen = require "moonshine.ui.screen"

function M.accept_line(text)
    local view = screen.current_view()
    local conv = view:get_conv()
    conv:send(text)
end

return M
