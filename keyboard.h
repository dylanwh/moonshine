/* vim: set ft=objc: */
#ifndef _SPOON_KEYMAP_H
#define _SPOON_KEYMAP_H
#include <objc/Object.h>
#include <slang.h>
#include <glib.h>

#include "config.h"

@interface Keyboard: Object
{
	GPtrArray *keysyms;
	SLkeymap_Type *keymap;
}

- bind: (char *)keyspec  to: (id)object;
- (void) processKey;

@end

#endif
