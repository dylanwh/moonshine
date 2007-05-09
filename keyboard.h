/* vim: set ft=objc: */
#ifndef _SPOON_KEYMAP_H
#define _SPOON_KEYMAP_H
#include <slang.h>
#include <glib.h>
#include "config.h"

typedef struct {
	SLkeymap_Type *keymap;
} Keyboard;

Keyboard *spoon_keyboard_new(void);
void spoon_keyboard_free(Keyboard *);

void spoon_keyboard_defkey(Keyboard *kbm, char *keyspec, char *keyname);
const char *spoon_keyboard_read(Keyboard *kb);

#endif
