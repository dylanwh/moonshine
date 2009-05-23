require "moonshine.prelude"
require "moonshine.protocol"
require "moonshine.prefs"
require "moonshine.ui"

local log  = require "moonshine.log"
local loop = require "moonshine.loop"

log.install()
loop.run()
