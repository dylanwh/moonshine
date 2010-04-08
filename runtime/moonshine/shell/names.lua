local screen = require "moonshine.ui.screen"

function cmd_names()
    local view = screen.current_view()
    if view.get_conversation then
        local conv = view:get_conversation()
        local users = conv:get_userlist()
        view:print_userlist(users)
        screen.render()
    end
end
