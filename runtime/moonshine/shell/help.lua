local shell = require "moonshine.shell"
local cmd   = {}

cmd.usage = "/help [--usage|-u] command"
cmd.help  = [[
help <command>

Shows help on commands. Try:
  /help help
]]

cmd.parse = shell.build_parser("usage|u", 1)

function cmd.exec(o, name)
	return shell.help(name)
end


return cmd
