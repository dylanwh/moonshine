/* vim: set ft=objc: */
#ifndef _SPOON_KEYBOARD_H
#define _SPOON_KEYBOARD_H
#include <slang.h>
#include <glib.h>
#include <lua.h>

#include "config.h"

typedef struct _Keyboard Keyboard;
Keyboard *keyboard_new(lua_State *L);
void keyboard_free(Keyboard *);
void keyboard_define(Keyboard *kb, char *keyspec, char *keyname);

//const char *keyboard_read(Keyboard *kb);

#endif
