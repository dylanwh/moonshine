require "moonshine.prelude"
local ui = require "moonshine.ui"

ui.init()
local loop = require "moonshine.loop"

name = "moonshine@irc.perl.org"
local account = new("purple.account", name, "prpl-irc")

account:set("port", 6667)
-- account:set_password("")
account:set_enabled(true)

loop:run()
