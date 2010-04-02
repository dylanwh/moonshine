local parseopt = require "moonshine.parseopt"
local log = require "moonshine.log"
local purple_account  = require "purple.account"
local purple_accounts = require "purple.accounts"

local parse = parseopt.build_parser {
    '#', -- name
    "protocol|P=s",
    "port|p=n",
}

function cmd_account(text)
    local opt, name = parse(text)
    assert(opt.protocol, "--protocol is required")
    local account = purple_account:new(name, opt.protocol)
    if opt.port then
        assert(type(opt.port) == 'number', "wrong type: " .. type(opt.port))
        account:set("port", opt.port)
    end
    purple_accounts.add(account)
    account:set_enabled(true)
    account:get_roomlist()
    log.debug("account added: %s", tostring(account))
end
