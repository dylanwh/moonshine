/* vim: set ft=objc: */
#ifndef _SPOON_KEYMAP_H
#define _SPOON_KEYMAP_H
#include <slang.h>
#include <glib.h>
#include "config.h"

typedef struct {
	union {
		char *name;
		char c;
		char *error;
	} data;
	enum { KEY_TYPE_NAME, KEY_TYPE_CHAR, KEY_TYPE_ERROR } type;
} Key;

typedef struct {
	GPtrArray     *names;
	SLkeymap_Type *keymap;
} Keyboard;

Keyboard *spoon_keyboard_new(void);
void spoon_keyboard_free(Keyboard *);

void spoon_keyboard_define(Keyboard *kbm, char *keyspec, char *keyname);
Key spoon_keyboard_read(Keyboard *kb);


#endif
