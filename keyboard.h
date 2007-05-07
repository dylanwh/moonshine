/* vim: set ft=objc: */
#ifndef _SPOON_KEYMAP_H
#define _SPOON_KEYMAP_H
#import <objc/Object.h>
#import <glib.h>

@interface Keyboard: Object
{
	GHashTable *map;
	id handler;
}

- define: (char *)name as: (int)key;
- bind: object;
- (void) processKey;

@end

#endif
