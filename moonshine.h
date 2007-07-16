/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_H__
#define __MOONSHINE_H__

#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/time.h>

#include <slang.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <event.h>

#define UNUSED __attribute__((unused))
#define THREAD __thread

/* typedefs */
typedef struct event Event;
typedef lua_State    LuaState;

/* util.c: utility functions */
void *emalloc(size_t size);
void efree(void *ptr);
void die(const char *errstr, ...);
bool moon_call(lua_State *L, const char *name, const char *sig, ...);
LuaState *moon_init(void);

/* term.c: low-level terminal routines. */
void term_init(void);
void term_resize(void);
void term_reset(void);

/* screen.c: high-level terminal routines. */
typedef struct Screen Screen;
Screen *screen_new(void);
void screen_refresh(Screen *scr);

#endif
