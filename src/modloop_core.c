#include <moonshine/config.h>
#include <moonshine/lua.h>

#define CLASS "moonshine.loop.core"

static int loop_new(LuaState *L)/*{{{*/
{
	GMainLoop **loop = ms_lua_newclass(L, CLASS, sizeof(GMainLoop *));
	*loop  = g_main_loop_new(NULL, FALSE); 
	return 1;
}/*}}}*/

static int loop_run(LuaState *L)/*{{{*/
{
	GMainLoop **loop = ms_lua_checkclass(L, CLASS, 1);
	g_main_loop_run(*loop);

	return 0;
}/*}}}*/

static int loop_quit(LuaState *L)/*{{{*/
{
	GMainLoop **loop = ms_lua_checkclass(L, CLASS, 1);
	g_main_loop_quit(*loop);

	return 0;
}/*}}}*/

static int loop_tostring(LuaState *L)/*{{{*/
{
	char buff[32];
  	sprintf(buff, "%p", ms_lua_toclass(L, CLASS, 1));
  	lua_pushfstring(L, "Loop (%s)", buff);
  	return 1;
}/*}}}*/

static int loop_gc(LuaState *L)/*{{{*/
{
	GMainLoop **loop = ms_lua_toclass(L, CLASS, 1);
	g_main_loop_unref(*loop);
	return 0;
}/*}}}*/

static const LuaLReg loop_methods[] = {/*{{{*/
	{"new", loop_new},
	{"run", loop_run},
	{"quit", loop_quit},
	{0, 0}
};/*}}}*/

static const LuaLReg loop_meta[] = {/*{{{*/
	{"__gc", loop_gc},
	{"__tostring", loop_tostring},
	{0, 0}
};/*}}}*/

int luaopen_moonshine_loop_core(LuaState *L)/*{{{*/
{
	ms_lua_class_register(L, CLASS, loop_methods, loop_meta);
	return 1;
}/*}}}*/
