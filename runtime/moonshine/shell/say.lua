local log             = require "moonshine.log"
local screen          = require "moonshine.ui.screen"

function cmd_say(text)
    local view = screen.current_view()
    if view.get_conversation then
        local conv = view:get_conversation()
        conv:send(text)
    else
        log.debug("/say %s", text)
    end
end
