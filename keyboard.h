/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 : */
#ifndef __MOONSHINE_KEYBOARD_H__
#define __MOONSHINE_KEYBOARD_H__
#include <slang.h>
#include <glib.h>
#include <lua.h>

#include "config.h"

/** \file keyboard.h
 * Watches keyboard for input.
 *
 * This module watches stdin and invokes the lua function on_keypress() 
 * for each keypress. */

/** Opaque struct representing the state associated with keyboard input. */
typedef struct Keyboard Keyboard;

/** Create a new keyboard object.
 *
 * This function allocates a new Keyboard object
 * and watches stdin for input.
 *
 * When input is ready, the lua function on_keypress(keyname) will be called.
 * keyname is either a single byte or an (ASCII) multibyte name, such as enter or backspace.
 **/
Keyboard *keyboard_new(lua_State *L);

/** Destroy a keyboard object. */
void keyboard_free(Keyboard *);

/** Associate a sequence of keys (keyspec) with a name (keyname).
 * keyspec can be something like:
 *
 * "^X"    -- control-x.
 *
 * "^(kD)" -- refers to a termcap/terminfo entry kD. (In this case, delete).
 *
 * "\r"    -- the carriage return character.*/
void keyboard_define(Keyboard *kb, char *keyspec, char *keyname);

//const char *keyboard_read(Keyboard *kb);

#endif
