module "moonshine.command.help"

usage = "/help [--usage|-u] command"
help  = [[
help <command>

Shows help on commands. Try:
  /help help
]]

function parse(line)
	return { usage = true }, line
end

function run(opt, name)
	local cmd = moonshine.command[name]

	signal.emit("print", cmd.help)
end
