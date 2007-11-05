#ifndef __MOONSHINE_HANDLE_H__
#define __MOONSHINE_HANDLE_H__
#include <glib.h>

#define HANDLE_ERROR g_quark_from_string("HandleError")

int luaopen_Handle(LuaState *L);

#endif
