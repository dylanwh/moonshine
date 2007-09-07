#include "moonshine.h"

#define TOPIC "Topic"

typedef struct {
	char *text;
} Topic;

static Topic *toTopic (LuaState *L, int index)
{
  	Topic *t = lua_touserdata(L, index);
  	if (t == NULL) luaL_typerror(L, index, TOPIC);
  	return t;
}

static Topic *checkTopic (LuaState *L, int index)
{
  	luaL_checktype(L, index, LUA_TUSERDATA);
  	Topic *t = luaL_checkudata(L, index, TOPIC);
  	if (t == NULL) luaL_typerror(L, index, TOPIC);
  	return t;
}

static Topic *pushTopic (LuaState *L, const char *text)
{
  	Topic *t = lua_newuserdata(L, sizeof(Topic));
  	luaL_getmetatable(L, TOPIC);
  	lua_setmetatable(L, -2);
  	t->text = g_strdup(text);
  	return t;
}

static int Topic_new (LuaState *L)
{
  	pushTopic(L);
  	return 1;
}

static int Topic_set(LuaState *L)
{
	Topic *t = checkTopic(L, 1);
	const char *text = luaL_checkstring(L, 2);

	if (t->text)
		g_free(t->text);

	t->text = g_strdup(text);
	return 0;
}

static int Topic_render(LuaState *L)
{
	Topic *t = checkTopic(L, 1);
	term_gotorc(0, 0);
	term_color_use("topic");
	term_write_char(t->text);
	return 0;
}

static const LuaLReg Topic_methods[] = {
  	{"new",    Topic_new},
  	{"set",    Topic_set},
  	{"render", Topic_render},
  	{0, 0}
};

static int Topic_gc (LuaState *L)
{
	Topic *t = toTopic(L, 1);
	g_free(t->text);
  	return 0;
}

static int Topic_tostring (LuaState *L)
{
  	Topic *t = toTopic(L, 1);
  	lua_pushstring(L, t->text);
  	return 1;
}

static const LuaLReg Topic_meta[] = {
  	{"__gc",       Topic_gc},
  	{"__tostring", Topic_tostring},
  	{0, 0}
};

void luaopen_Topic(LuaState *L)
{
	moon_class_create(L, TOPIC, Topic_methods, Topic_meta);
}
