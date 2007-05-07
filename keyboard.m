#include <objc/encoding.h>
#include <slang.h>
#include "keyboard.h"

@implementation Keyboard
- init
{
	map     = g_hash_table_new(g_direct_hash, g_direct_equal);
	handler = self;
	[self define: "key_error"     as: SL_KEY_ERR ];
	[self define: "key_up"        as: SL_KEY_UP ];
	[self define: "key_down"      as: SL_KEY_DOWN ];
	[self define: "key_left"      as: SL_KEY_LEFT ];
	[self define: "key_right"     as: SL_KEY_RIGHT ];
	[self define: "key_ppage"     as: SL_KEY_PPAGE ];
	[self define: "key_npage"     as: SL_KEY_NPAGE ];
	[self define: "key_home"      as: SL_KEY_HOME ];
	[self define: "key_end"       as: SL_KEY_END ];
	[self define: "key_a1"        as: SL_KEY_A1 ];
	[self define: "key_a3"        as: SL_KEY_A3 ];
	[self define: "key_b2"        as: SL_KEY_B2 ];
	[self define: "key_c1"        as: SL_KEY_C1 ];
	[self define: "key_c3"        as: SL_KEY_C3 ];
	[self define: "key_redo"      as: SL_KEY_REDO ];
	[self define: "key_undo"      as: SL_KEY_UNDO ];
	[self define: "key_backspace" as: SL_KEY_BACKSPACE ];
	[self define: "key_enter"     as: SL_KEY_ENTER ];
	[self define: "key_enter"     as: '\r' ];
	[self define: "key_ic"        as: SL_KEY_IC ];
	[self define: "key_delete"    as: SL_KEY_DELETE ];
	return self;
}

- define: (char *)name as: (int)key
{
	g_hash_table_insert(map, GINT_TO_POINTER(key), name);
	return self;
}

- bind: obj
{
	handler = obj;
	return self;
}

- (void) processKey
{
	int key    = SLkp_getkey();
	char *name = g_hash_table_lookup(map, GINT_TO_POINTER(key));
	SEL sel;

	if (name && handler) {
		sel = sel_get_uid(name);
		if (sel) {
			[handler perform: sel];
		}
	} else {
		sel = @selector(key_other:);
		if (sel) {
			IMP func = [handler methodFor: sel];
			if (func) {
				func(handler, (SEL)sel, key);
			} else {
				[handler doesNotRecognize: sel];
			}
		}
	}
}

@end
