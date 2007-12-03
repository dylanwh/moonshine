#include "moon.h"
#include "term.h"
#include "config.h"
#include "util.h"

typedef struct {
	GCompletion *cmp;
} Completion;

static int Completion_new(LuaState *L)
{
	Completion *c    = moon_newclass(L, "Completion", sizeof(Completion));
	c->cmp           = g_completion_new(NULL);
	return 1;
}

static int Completion_add(LuaState *L)
{
	Completion *c = moon_checkclass(L, "Completion", 1);
	int max = lua_gettop(L);
	GList *list = NULL;

	for (int i = 2; i <= max; i++) {
		const char *item = luaL_checkstring(L, i);
		list = g_list_prepend(list, g_strdup(item));
	}

	g_completion_add_items(c->cmp, list);
	return 0;
}

static int Completion_complete(LuaState *L)
{
	Completion *c      = moon_checkclass(L, "Completion", 1);
	const char *prefix = luaL_checkstring(L, 2);
	char *newprefix    = NULL;
	GList *items = g_completion_complete_utf8(c->cmp, prefix, &newprefix);
	lua_pushstring(L, newprefix);
	g_free(newprefix);
	lua_newtable(L);

	for (int i = 1; items; items = g_list_next(items)) {
		lua_pushstring(L, items->data);
		lua_rawseti(L, -2, i++);
	}
	return 2;
}

static int Completion_tostring(LuaState *L)
{
	char buff[32];
  	sprintf(buff, "%p", moon_toclass(L, "Completion", 1));
  	lua_pushfstring(L, "Completion (%s)", buff);
  	return 1;
}

static int Completion_gc(LuaState *L)
{
	Completion *c = moon_toclass(L, "Completion", 1);
	g_completion_free(c->cmp);
	return 0;
}

static const LuaLReg Completion_methods[] = {
	{"new", Completion_new},
	{"add", Completion_add},
	{"complete", Completion_complete},
	{0, 0}
};

static const LuaLReg Completion_meta[] = {
	{"__gc", Completion_gc},
	{"__tostring", Completion_tostring},
	{0, 0}
};

int luaopen_completion(LuaState *L)
{
	moon_class_register(L, "Completion", Completion_methods, Completion_meta);
	return 1;
}
