local parseopt = require "moonshine.parseopt"
local log = require "moonshine.log"
local purple_account  = require "purple.account"

local get_subcmd = parseopt.build_parser { '#' }

local subcmd = {}
local parser = {
    add = parseopt.build_parser {
        '#', -- username
        '#', -- protocol
        'port|p=n',
        'password|pass|P=s',
        'enable|e',
    },
    remove  = parseopt.build_parser { '#', '#' },
    enable  = parseopt.build_parser { '#', '#' },
    disable = parseopt.build_parser { '#', '#' },
}

function cmd_account(text)
    local _, name, rest = get_subcmd(text)
    if name then
        if parser[name] then
            subcmd[name]( parser[name](rest) )
        else
            subcmd[name](rest)
        end
    else
        subcmd.list()
    end
end

function subcmd.add(opt, username, protocol)
    assert(username and protocol, "usage: /account add [options] username protocol")
    log.debug("going to add new account %s (%s)", username, protocol)
    local account = purple_account:new(username, protocol)
    if opt.port then
        account:set("port", tonumber(opt.port))
    end
    if opt.password then
        account:set_password(opt.password)
    end
    if opt.enable then
        account:set_enabled(true)
    end
    log.debug("added account %s (%s)", username, protocol)
end

function subcmd.remove(_, username, protocol)
    assert(username and protocol, "usage: /account remove [options] username protocol")
    log.debug("going to remove account %s (%s)", username, protocol)
    local account = purple_account:find(username, protocol)
    account:delete()
    log.debug("removed account %s (%s)", username, protocol)
end

function subcmd.enable(_, username, protocol)
    assert(username and protocol, "usage: /account enable [options] username protocol")
    log.debug("going to enable account %s (%s)", username, protocol)
    subcmd.list()
    local account = assert(purple_account:find(username, protocol))
    account:set_enabled(true)
    log.debug("enabled account %s (%s)", username, protocol)
end

function subcmd.disable(_, username, protocol)
    assert(username and protocol, "usage: /account disable [options] username protocol")
    log.debug("going to disable account %s (%s)", username, protocol)
    local account = purple_account:find(username, protocol)
    account:set_enabled(false)
    log.debug("disabled account %s (%s)", username, protocol)
end


function subcmd.list()
    local list = purple_account:get_all()
    if #list == 0 then
        log.debug("no accounts")
    end
    for i, account in ipairs(list) do
        log.debug("account %d: %s (%s)", i, account:get_username(), account:get_protocol_id())
    end
end
