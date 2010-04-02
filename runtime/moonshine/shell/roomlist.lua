local parseopt = require "moonshine.parseopt"
local log = require "moonshine.log"
local purple_account  = require "purple.account"
local purple_accounts = require "purple.accounts"

local parse = parseopt.build_parser {
    '#', -- name
    'protocol|P=s',
}

function cmd_roomlist(text)
    local opt, name = parse(text)
    assert(opt.protocol, "--protocol is required")
    local account = purple_accounts.find(name, opt.protocol)
    log.debug("getting roomlist")
    account:get_roomlist()
end
