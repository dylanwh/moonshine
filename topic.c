#include "moon.h"
#include "term.h"

typedef struct {
	char *text;
} Topic;

static int Topic_new(LuaState *L)
{
	const char *text = luaL_optstring(L, 1, ""); 
	Topic *t         = moon_newclass(L, "Topic", sizeof(Topic));
	t->text          = g_strdup(text);
	return 1;
}

static int Topic_set(LuaState *L)
{
	Topic *t         = moon_checkclass(L, "Topic", 1);
	const char *text = luaL_checkstring(L, 2);
	g_free(t->text);
	t->text = g_strdup(text);
	return 0;
}

static int Topic_render(LuaState *L)
{
	Topic *t = moon_checkclass(L, "Topic", 1);
	term_goto(0, 0);
	term_color_use("topic");
	term_erase_eol();
	term_write_chars(t->text);
	term_color_use("default");
	return 0;
}

static int Topic_tostring(LuaState *L)
{
	char buff[32];
  	sprintf(buff, "%p", moon_toclass(L, "Topic", 1));
  	lua_pushfstring(L, "Topic (%s)", buff);
  	return 1;
}

static int Topic_gc(LuaState *L)
{
	Topic *t = moon_toclass(L, "Topic", 1);
	g_free(t->text);
	return 0;
}

static const LuaLReg Topic_methods[] = {
	{"new", Topic_new},
	{"render", Topic_render},
	{"set", Topic_set},
	{0, 0}
};

static const LuaLReg Topic_meta[] = {
	{"__gc", Topic_gc},
	{"__tostring", Topic_tostring},
	{0, 0}
};

int luaopen_Topic(LuaState *L)
{
	moon_class_register(L, "Topic", Topic_methods, Topic_meta);
	return 1;
}
