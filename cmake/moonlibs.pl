#!/usr/bin/perl
use strict;
use warnings;

my $head = <<HEAD;
static const LuaLReg lualibs[] = {
	{"", luaopen_base},
	{LUA_LOADLIBNAME, luaopen_package},
	{LUA_TABLIBNAME,  luaopen_table},
	{LUA_IOLIBNAME,   luaopen_io},
	{LUA_OSLIBNAME,   luaopen_os},
	{LUA_STRLIBNAME,  luaopen_string},
	{LUA_MATHLIBNAME, luaopen_math},
	{LUA_DBLIBNAME,   luaopen_debug},
HEAD

my $foot = <<FOOT;
	{NULL, NULL}
};
FOOT

my @functions = ();
while (<ARGV>) {
	if (/int (luaopen_\w+)\(LuaState \*L\)/) {
		push @functions, $1;
	}
}


foreach my $func (@functions) {
	print "int $func(LuaState *L);\n";
}

print $head;
foreach my $func (@functions) {
	$func =~ /luaopen_(\w+)/;
	my $name = $1;
	print qq(\t{"$name", $func},\n);
}
print $foot;


