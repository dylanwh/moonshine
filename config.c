#include "moon.h"
#include "term.h"
#include "config.h"
#include "util.h"

typedef struct {
	GKeyFile *keyfile;
} Config;

static int Config_open(LuaState *L)
{
	const char *filename = luaL_optstring(L, 2, "");
	GError *error  = NULL;
	const char *dirs[] = {
		".",
		g_get_home_dir (),
		g_get_user_config_dir(),
		NULL
	};
	Config *cfg            = moon_newclass(L, "Config", sizeof(Config));
	cfg->keyfile           = g_key_file_new();

	g_key_file_set_list_separator(cfg->keyfile, ',');
	gboolean ok = g_key_file_load_from_dirs(cfg->keyfile, filename, dirs, NULL,
			G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error);

	if (ok) return 1;
	else {
		lua_pop(L, 1);
		lua_pushnil(L);
		moon_pusherror(L, error);
		g_error_free(error);
		return 2;
	}
}

static int Config_get(LuaState *L)
{
	Config *cfg       = moon_checkclass(L, "Config", 1);
	const char *group = luaL_checkstring(L, 2);
	const char *key   = luaL_checkstring(L, 3);
	GError *error = NULL;

	char *value = g_key_file_get_locale_string(cfg->keyfile, group, key, NULL, &error);
	if (value) {
		lua_pushstring(L, value);
		g_free(value);
		return 1;
	} else {
		lua_pushnil(L);
		moon_pusherror(L, error);
		g_error_free(error);
		return 2;
	}
}

static int Config_set(LuaState *L)
{
	Config *cfg       = moon_checkclass(L, "Config", 1);
	const char *group = luaL_checkstring(L, 2);
	const char *key   = luaL_checkstring(L, 3);
	const char *value = luaL_checkstring(L, 4);
	g_key_file_set_value(cfg->keyfile, group, key, value);
	return 0;
}

static int Config_dump(LuaState *L)
{
	Config *cfg       = moon_checkclass(L, "Config", 1);
	gsize len;

	char *data = g_key_file_to_data(cfg->keyfile, &len, NULL);
	lua_pushlstring(L, data, len);
	g_free(data);
	return 1;
}


static int Config_tostring(LuaState *L)
{
	char buff[32];
  	sprintf(buff, "%p", moon_toclass(L, "Config", 1));
  	lua_pushfstring(L, "Config (%s)", buff);
  	return 1;
}

static int Config_gc(LuaState *L)
{
	Config *cfg = moon_toclass(L, "Config", 1);
	g_key_file_free(cfg->keyfile);
	return 0;
}

static const LuaLReg Config_methods[] = {
	{"open", Config_open},
	{"get", Config_get},
	{"set", Config_set},
	{"dump", Config_dump},
	{0, 0}
};

static const LuaLReg Config_meta[] = {
	{"__gc", Config_gc},
	{"__tostring", Config_tostring},
	{0, 0}
};

int luaopen_config(LuaState *L)
{
	moon_class_register(L, "Config", Config_methods, Config_meta);
	return 1;
}
