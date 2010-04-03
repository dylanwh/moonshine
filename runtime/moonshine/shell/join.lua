local parseopt        = require "moonshine.parseopt"
local log             = require "moonshine.log"
local purple_account  = require "purple.account"
local purple_accounts = require "purple.accounts"

local parse = parseopt.build_parser {
    '#', -- name
    "protocol|P=s",
    "account|A=s",
}

function cmd_join(text)
    local opt, roomname = parse(text)
    assert(opt.protocol, "--protocol is required")
    assert(opt.account,  "--account is required")

    local account = purple_accounts.find(opt.account, opt.protocol)
    account:join_chat(roomname)
end
