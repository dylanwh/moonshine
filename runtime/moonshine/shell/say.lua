local parseopt = require "moonshine.parseopt"
local log = require "moonshine.log"
local purple_account  = require "purple.account"
local purple_accounts = require "purple.accounts"

local parse = parseopt.build_parser {
    "foo"
}

function cmd_say(text)
    local opt, text = parse(text)
    log.debug("say: %s", text)
end
