/* vim: set ft=objc: */
#ifndef _SPOON_KEYMAP_H
#define _SPOON_KEYMAP_H
#include <slang.h>
#include <glib.h>
#include "config.h"

typedef struct {
	SLkeymap_Type *keymap;
	GHashTable *table; /* callbacks */
	Closure *fallback;
} Keyboard;

Keyboard *spoon_keyboard_new(void);
void spoon_keyboard_free(Keyboard *);

void spoon_keyboard_define(Keyboard *kb, char *keyspec, char *keyname);
void spoon_keyboard_bind(Keyboard *kb, char *keyname, Closure *c);

const char *spoon_keyboard_read(Keyboard *kb);

#endif
