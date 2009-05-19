#include <moonshine/parseopt.h>
#include <moonshine/config.h>
#include <moonshine/lua.h>

static MSParseOptStatus short_option(gpointer baton, const char opt, const char *argument, UNUSED const char *start)/*{{{*/
{
	char opt_str[2] = "-\0";
	MSLuaRef *R = baton;
	LuaState *L = ms_lua_pushref(R);

	opt_str[0] = opt;
	lua_pushstring(L, opt_str);
	lua_pushstring(L, argument);
	lua_call(L, 2, 1);
	return lua_tointeger(L, -1);
}/*}}}*/

static MSParseOptStatus long_option(gpointer baton, const char *opt, const char *argument, UNUSED const char *start)/*{{{*/
{
	MSLuaRef *R = baton;
	LuaState *L = ms_lua_pushref(R);
	lua_pushstring(L, opt);
	lua_pushstring(L, argument);
	lua_call(L, 2, 1);
	return lua_tointeger(L, -1);
}/*}}}*/

static MSParseOptStatus literal_option(gpointer baton, const char *literal, UNUSED const char *start)/*{{{*/
{
	MSLuaRef *R = baton;
	LuaState *L = ms_lua_pushref(R);

	lua_pushnil(L);
	lua_pushstring(L, literal);
	lua_call(L, 2, 1);
	return lua_tointeger(L, -1);
}/*}}}*/

static MSParseOptCallbacks parseopt_cb = {
	.shortopt = short_option,
	.longopt  = long_option,
	.literalopt = literal_option
};

static int parseopt_parse(LuaState *L)
{
	const char *argstr = luaL_checkstring(L, 1);
	MSLuaRef *R        = ms_lua_ref(L, 2);
	const char *result = ms_parseopt_parse(R, argstr, &parseopt_cb);
	ms_lua_unref(R);
	lua_pushstring(L, result);
	return 1;
}

static LuaLReg functions[] = {
	{"parse",  parseopt_parse },
	{ 0, 0 },
};

int luaopen_moonshine_parseopt_core(LuaState *L)/*{{{*/
{
	lua_newtable(L);
	lua_pushinteger(L, MS_PARSEOPT_STOP);
	lua_setfield(L, -2, "STOP");
	lua_pushinteger(L, MS_PARSEOPT_NOARG);
	lua_setfield(L, -2, "NOARG");
	lua_pushinteger(L, MS_PARSEOPT_EATARG);
	lua_setfield(L, -2, "EATARG");
	luaL_register(L, NULL, functions);
	return 1;
}/*}}}*/
