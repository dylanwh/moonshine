#include <moonshine/purple.h>

void ms_purple_push_cbuddy(LuaState *L, PurpleConvChatBuddy *cbuddy)
{
    g_assert(cbuddy != NULL);

    lua_createtable(L, 0, 4);
    lua_pushstring(L, cbuddy->name);
    lua_setfield(L, -2, "name");
    lua_pushstring(L, cbuddy->alias);
    lua_setfield(L, -2, "alias");
    lua_pushboolean(L, cbuddy->buddy);
    lua_setfield(L, -2, "buddy");
    lua_newtable(L);

    if (cbuddy->flags & PURPLE_CBFLAGS_VOICE) {
        lua_pushboolean(L, TRUE);
        lua_setfield(L, -2, "voice");
    }
    if (cbuddy->flags & PURPLE_CBFLAGS_HALFOP) {
        lua_pushboolean(L, TRUE);
        lua_setfield(L, -2, "halfop");
    }
    if (cbuddy->flags & PURPLE_CBFLAGS_OP) {
        lua_pushboolean(L, TRUE);
        lua_setfield(L, -2, "op");
    }
    if (cbuddy->flags & PURPLE_CBFLAGS_FOUNDER) {
        lua_pushboolean(L, TRUE);
        lua_setfield(L, -2, "founder");
    }
    if (cbuddy->flags & PURPLE_CBFLAGS_TYPING) {
        lua_pushboolean(L, TRUE);
        lua_setfield(L, -2, "founder");
    }
    lua_setfield(L, -2, "flags");
}
