#include "moonshine/config.h"
#include "moonshine/log.h"
#include "moonshine/lua.h"

#include <glib.h>

static void log_default_handler(const char *log_domain, GLogLevelFlags log_level, const char *message, gpointer user_data)/*{{{*/
{
	LuaState *L = ms_lua_pushref((MSLuaRef *)user_data);

	/* argument number 1 */
	if (log_domain)
		lua_pushstring(L, log_domain); 
	else
		lua_pushstring(L, "");


	/* argument number 2 */
	switch (log_level & G_LOG_LEVEL_MASK) {
		case G_LOG_LEVEL_ERROR:
			lua_pushstring(L, "ERROR");
			break;
		case G_LOG_LEVEL_CRITICAL:
			lua_pushstring(L, "CRITICAL");
			break;
		case G_LOG_LEVEL_WARNING:
			lua_pushstring(L, "WARNING");
			break;
		case G_LOG_LEVEL_MESSAGE:
			lua_pushstring(L, "MESSAGE");
			break;
		case G_LOG_LEVEL_INFO:
			lua_pushstring(L, "INFO");
			break;
		case G_LOG_LEVEL_DEBUG:
			lua_pushstring(L, "DEBUG");
			break;
		default:
			lua_pushstring(L, "WTF");
			break;
	}

	/* argument number 3 */
	lua_pushstring(L, message);

	if (lua_pcall(L, 3, 0, 0)) {
		g_error("moonshine error in default log handler: %s", lua_tostring(L, -1));
	}
}/*}}}*/

static int log_set_default_handler(LuaState *L)/*{{{*/
{
	static gboolean did_replay = FALSE;
	MSLuaRef *func = ms_lua_ref_checktype(L, 1, LUA_TFUNCTION);
	MSLog *log     = (MSLog *)ms_lua_stash_get(L, "log");

	g_assert(log);

	if (!did_replay)
		ms_log_replay(log, log_default_handler, (gpointer)func);

	did_replay = TRUE;
	return 0;
}/*}}}*/

static int log_print(LuaState *L)
{
	const char *domain;
	const char *level   = luaL_checkstring(L, 2);
	const char *message = luaL_checkstring(L, 3);

	if (lua_isnil(L, 1)) domain = G_LOG_DOMAIN;
	else                 domain = luaL_checkstring(L, 1);

	GLogLevelFlags flag;
 	if (strcasecmp(level, "ERROR") == 0)         flag = G_LOG_LEVEL_ERROR;
 	else if (strcasecmp(level, "CRITICAL") == 0) flag = G_LOG_LEVEL_CRITICAL;
 	else if (strcasecmp(level, "WARNING") == 0)  flag = G_LOG_LEVEL_WARNING;
 	else if (strcasecmp(level, "MESSAGE") == 0)  flag = G_LOG_LEVEL_MESSAGE;
 	else if (strcasecmp(level, "INFO") == 0)     flag = G_LOG_LEVEL_INFO;
 	else if (strcasecmp(level, "DEBUG") == 0)    flag = G_LOG_LEVEL_DEBUG;
 	else return luaL_error(L, "Unknown log level: %s", level);
 	
 	g_log(domain, flag, "%s", message);

 	return 0;
}

static LuaLReg functions[] = {/*{{{*/
	{ "set_default_handler",  log_set_default_handler },
	{ "print",                log_print               },
	{ 0, 0 },
};/*}}}*/

int luaopen_moonshine_log_core(LuaState *L)/*{{{*/
{
	lua_newtable(L);
	luaL_register(L, NULL, functions);
	g_debug("This is a test of the emergency broadcast system.");
	return 1;
}/*}}}*/
