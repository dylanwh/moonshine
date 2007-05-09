#include <objc/encoding.h>
#include "keyboard.h"

inline static int KEYSYM(guint i) { 
	return i + 0x1000;
}
inline static int find_free_index(GPtrArray *a)
{
	for (int i = 0; i < a->len; i++) {
		if (g_ptr_array_index(a, i) == NULL)
			return i;
	}

	guint size = a->len;
	g_ptr_array_set_size(a, size + 1);
	return size;
}


@implementation Keyboard
- init
{
	char k[4] = {0, 0, 0, 0};
	keymap  = SLang_create_keymap("default", NULL);
	keysyms = g_ptr_array_new();
	for (int i = 0; i < 256; i++) {
		k[0] = i;
		SLkm_define_keysym(k, i, keymap);
	}
	return self;
}

- bind: (char *)keyspec to: object
{
	int i = find_free_index(keysyms);
	SLkm_define_keysym(keyspec, KEYSYM(i), keymap);
	g_ptr_array_index(keysyms, i) = (gpointer) object;
	return self;
}

- (void) processKey
{
	UNUSED int key    = SLkp_getkey();
	UNUSED SEL sel;
/*
	if (name && handler) {
		sel = sel_get_uid(name);
		if (sel) {
			[handler perform: sel];
		}
	} else {
		sel = @selector(keypress:);
		if (sel) {
			IMP func = [handler methodFor: sel];
			if (func) {
				func(handler, (SEL)sel, key);
			} else {
				[handler doesNotRecognize: sel];
			}
		}
	}*/
}

@end
