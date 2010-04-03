local View    = require "moonshine.ui.view"
local ConView = View:clone()

function ConView:__init(init)
    View.__init(self, init)
    self._conversation = assert(init.conversation, "conversation is required")
end

function ConView:get_conversation()
    return self._conversation
end

function ConView:render(...)
    local ok, err = pcall(function ()
        if self._conversation:get_type() == 'chat' then
            self:update_topic( self._conversation:get_topic() )
        end
    end)
    if not LOG and not ok then
        local log = require "moonshine.log"
        LOG=true
        log.critical("%s", err)
        LOG=false
    end
    View.render(self, ...)
end

return ConView
