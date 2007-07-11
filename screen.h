/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_SCREEN_H__
#define __MOONSHINE_SCREEN_H__
#include <lua.h>
#include <glib.h>

#include "buffer.h"

typedef struct Screen Screen;
Screen *screen_new(lua_State *L);
void screen_print(Screen *scr, GString *msg);
void screen_refresh(Screen *scr);

#endif
