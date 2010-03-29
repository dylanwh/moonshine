#include "moonshine/config.h"
#include "moonshine/lua.h"
#include <glib.h>

/* the following is for the bd_-code in push_paths() */
#include <stdlib.h>

#define MS_LUA_BACKREF "BACKREF"

/* Stash-related functions {{{*/
void ms_lua_stash_set(LuaState *L, const char *name, gpointer user_data)/*{{{*/
{
    char *key = g_strconcat("ms_stash_", name, NULL);
    lua_pushlightuserdata(L, user_data);
    lua_setfield(L, LUA_REGISTRYINDEX, key);
    g_free(key);
}/*}}}*/

gpointer ms_lua_stash_get(LuaState *L, const char *name)/*{{{*/
{
    char *key = g_strconcat("ms_stash_", name, NULL);
    lua_getfield(L, LUA_REGISTRYINDEX, key);

    gpointer result = NULL;
    if (lua_islightuserdata(L, -1)) {
        result = lua_touserdata(L, -1);
    }
    else {
        g_assert(lua_isnil(L, -1));
    }

    lua_pop(L, 1);
    g_free(key);
    return result;
}/*}}}*/
/* }}} */

/* Backref-related functions {{{*/
void ms_lua_backref_set(LuaState *L, gpointer ptr, int idx)
{
    lua_pushvalue(L, idx);                              // push object
    lua_getfield(L, LUA_REGISTRYINDEX, MS_LUA_BACKREF); // push table
    g_assert(!lua_isnil(L, -1));
    lua_insert(L, -2);                                 // object table -- table object
    g_assert(lua_type(L, -2) == LUA_TTABLE);
    g_assert(lua_type(L, -1) == LUA_TUSERDATA);

    lua_pushlightuserdata(L, ptr);                      // push ptr
    lua_insert(L, -2);                                 // object ptr -- ptr object

    g_assert(lua_type(L, -3) == LUA_TTABLE);
    g_assert(lua_type(L, -2) == LUA_TLIGHTUSERDATA);
    g_assert(lua_type(L, -1) == LUA_TUSERDATA);

    lua_settable(L, -3); // table[ptr] = object
    lua_pop(L, 1);       // remove table from stack. stack is now in original state.
}

void ms_lua_backref_push(LuaState *L, gpointer ptr)
{
    lua_getfield(L, LUA_REGISTRYINDEX, MS_LUA_BACKREF); // push table
    lua_pushlightuserdata(L, ptr);                      // push ptr
    lua_gettable(L, -2);                                // pop ptr; push table[ptr]
    lua_remove(L, -2);                                  // remove table from stack.
}

void ms_lua_backref_unset(LuaState *L, gpointer ptr)
{
    lua_getfield(L, LUA_REGISTRYINDEX, MS_LUA_BACKREF); // push table
    lua_pushlightuserdata(L, ptr);                      // push ptr
    lua_pushnil(L);                                     // push nil
    lua_settable(L, -3);                                // table[ptr] = nil
    lua_pop(L, 1);                                      // pop table
}

/* }}} */

/* Class-related functions {{{ */
gpointer ms_lua_toclass(LuaState *L, const char *name, int index)/*{{{*/
{
    gpointer p = lua_touserdata(L, index);
    if (p == NULL) luaL_typerror(L, index, name);
    return p;
}/*}}}*/

gpointer ms_lua_checkclass(LuaState *L, const char *name, int index)/*{{{*/
{
    luaL_checktype(L, index, LUA_TUSERDATA);
    gpointer p = luaL_checkudata(L, index, name);
    if (p == NULL) luaL_typerror(L, index, name);
    return p;
}/*}}}*/

gpointer ms_lua_newclass(LuaState *L, const char *name, gsize size)/*{{{*/
{
    gpointer p = lua_newuserdata(L, size);
    luaL_getmetatable(L, name);
    lua_setmetatable(L, -2);
    return p;
}/*}}}*/

void ms_lua_class_register(LuaState *L, const char *name, const LuaLReg methods[], const LuaLReg meta[])/*{{{*/
{
    lua_newtable(L);
    luaL_register(L, NULL, methods);  /* create methods table, do not add it to the globals */

    luaL_newmetatable(L, name);      /* create metatable for Buffer, and add it
                                         to the Lua registry */
    luaL_register(L, NULL, meta);      /* fill metatable */

    lua_pushliteral(L, "__index");    /* Set meta.__index = methods */
    lua_pushvalue(L, -3);             /* dup methods table*/
    lua_rawset(L, -3);                /* metatable.__index = methods */

    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, -3);             /* dup methods table*/
    lua_rawset(L, -3);                /* hide metatable: metatable.__metatable = methods */

    lua_remove(L, -1);
}/*}}}*/
/*}}}*/

void ms_lua_use_env(LuaState *L)/*{{{*/
{
    lua_newtable(L);
    lua_replace(L, LUA_ENVIRONINDEX);
}/*}}}*/

gboolean ms_lua_require(LuaState *L, const char *name)/*{{{*/
{
    int len = strlen("ms_lua_require()") + strlen(name) + 1;
    char buf[len];
    g_snprintf(buf, len, "ms_lua_require(%s)", name);

    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    return ms_lua_call(L, 1, 0, buf);
}/*}}}*/

void ms_lua_module(LuaState *L, const LuaLReg functions[])/*{{{*/
{
    lua_newtable(L);
    luaL_register(L, NULL, functions);
}/*}}}*/

void ms_lua_preload(LuaState *L, const char *name, lua_CFunction func)/*{{{*/
{
    g_return_if_fail(L != NULL);
    g_return_if_fail(name != NULL);
    g_return_if_fail(func != NULL);

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, func);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
}/*}}}*/

static void push_paths /*{{{*/
    (LuaState *L, const char *defpath, const char *envname, const char *ext)
{
    const char *envpath = g_getenv(envname);
    const char *endseg;
    int concat_ct = 0;
    if (!envpath || !*envpath)
        envpath = getenv("MOONSHINE_PATH");
    if (!envpath || !*envpath)
        envpath = "";

    /* prepend envpath to our search string. also, helpfully add ?.lua and ?.so
     * if missing
     *
     * First, though, we need to break into path segments.
     */
    do {
        const char *segstr;
        endseg = strchr(envpath, ';');

        if (endseg)
            lua_pushlstring(L, envpath, endseg - envpath);
        else
            lua_pushstring(L, envpath);

        concat_ct++;

        segstr = lua_tostring(L, -1);
        if (!strchr(segstr, '?')) {
            lua_pushstring(L, "/?");
            lua_pushstring(L, ext);
            concat_ct += 2;
        }
        lua_pushstring(L, ";");
        concat_ct++;

        envpath = endseg + 1;
    } while (endseg);

    lua_pushstring(L, defpath);
    concat_ct++;

    lua_concat(L, concat_ct);
}/*}}}*/

static void init_paths(LuaState *L)/*{{{*/
{
    /* push the global package onto the stack */
    lua_getglobal(L, "package");

    /* Assign package.cpath = modules */
    push_paths(L, MOONSHINE_PATH ";" LUA_PATH_DEFAULT,
            "MOONSHINE_RUNTIME_PATH", ".lua");
    lua_setfield(L, -2, "path");

    /* Assign package.cpath = modules */
    push_paths(L, MOONSHINE_CPATH ";" LUA_CPATH_DEFAULT,
            "MOONSHINE_MODULES_PATH", MOONSHINE_SO);
    lua_setfield(L, -2, "cpath");

    /* remove package from the stack. */
    lua_pop(L, 1);
}/*}}}*/

inline static void ms_lua_newtable_weak(LuaState *L)/*{{{*/
{
    lua_newtable(L);               // push table (t).
    lua_newtable(L);               // push metatable (mt).
    lua_pushstring(L, "v");        // push "v"
    lua_setfield(L, -2, "__mode"); // mt.__mode = "v"
    lua_setmetatable(L, -2);       // setmetatable(t, mt)
}/*}}}*/

LuaState *ms_lua_newstate(void)/*{{{*/
{
    LuaState *L  = luaL_newstate();
    luaL_openlibs(L);

    lua_newtable(L);
    lua_pushstring(L, MOONSHINE_VERSION);
    lua_setglobal(L, "MOONSHINE_VERSION");
    init_paths(L);

    ms_lua_newtable_weak(L);
    lua_setfield(L, LUA_REGISTRYINDEX, MS_LUA_BACKREF);

    return L;
}/*}}}*/
