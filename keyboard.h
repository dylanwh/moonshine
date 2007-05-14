/* vim: set ft=objc: */
#ifndef _SPOON_KEYBOARD_H
#define _SPOON_KEYBOARD_H
#include <slang.h>
#include <glib.h>

#include "config.h"
#include "closure.h"

typedef struct {
	SLkeymap_Type *keymap;
	GHashTable *table; /* callbacks */
	Closure *fallback;
	GIOChannel *channel;
} Keyboard;

Keyboard *keyboard_new(void);
void keyboard_free(Keyboard *);

void keyboard_define(Keyboard *kb, char *keyspec, char *keyname);
void keyboard_bind(Keyboard *kb, char *keyname, Closure *c);
void keyboard_bind_fallback(Keyboard *kb, Closure *c);

//const char *keyboard_read(Keyboard *kb);

#endif
