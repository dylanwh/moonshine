/* vim: set ft=objc: */
#ifndef _SPOON_KEYMAP_H
#define _SPOON_KEYMAP_H
#import <objc/Object.h>
#import <glib.h>

@interface KeyMap: Object
{
	GHashTable *map;
	id handler;
}

- (void) define: (char *)name   as: (int)key;
- (void) bind: object;
- (void) processKey;

@end

#endif
