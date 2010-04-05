local View    = require "moonshine.ui.view"
local ConView = View:clone()

function ConView:__init(init)
    local conv = assert(init.conversation, "conversation is required")
    init.name = conv:get_name()
    View.__init(self, init)
    self._conversation = conv
    self:update_topic( conv:get_topic() )
end

function ConView:close()
    local conv = self._conversation
    self._conversation = nil
    conv:destroy()
end

function ConView:get_conversation()
    return self._conversation
end

return ConView
