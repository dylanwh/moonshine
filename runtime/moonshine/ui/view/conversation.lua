local View    = require "moonshine.ui.view"
local ConView = View:clone()

function ConView:__init(init)
    View.__init(self, init)
    self._conversation = assert(init.conversation, "conversation is required")
end

function ConView:get_conversation()
    return self._conversation
end

return ConView
