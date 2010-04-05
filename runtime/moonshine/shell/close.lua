local log             = require "moonshine.log"
local screen          = require "moonshine.ui.screen"

function cmd_close(text)
    local view = screen.current_view()
    if view == screen.find_view(1) then return end
    view:close()
end
