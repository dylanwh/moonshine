#include <string.h>

#include "config.h"
#include "keyboard.h"

const int KEYSYM_BASE = 0x256;
inline static void init_keymap(Keyboard *kb);
inline static guint find_free_index(GPtrArray *a);
inline static Key error_key(char *msg);

Keyboard *spoon_keyboard_new(void)
{
	Keyboard *kb = g_new(Keyboard, 1);
	kb->names    = g_ptr_array_new();
	kb->keymap   = SLang_create_keymap("default", NULL);
	init_keymap(kb);
	return kb;
}

void spoon_keyboard_defkey(Keyboard *kb, char *spec, char *name)
{
	guint idx = find_free_index(kb->names);
	int keysym = KEYSYM_BASE + idx;
	g_ptr_array_index(kb->names, idx) = (gpointer) g_strdup(name);
	SLkm_define_keysym(spec, keysym, kb->keymap);
}

Key spoon_keyboard_read(Keyboard *kb)
{
	SLang_Key_Type *key = SLang_do_key(kb->keymap, (int (*)(void)) SLang_getkey);

	if (key == NULL || key->type != SLKEY_F_KEYSYM) {
		SLang_flush_input();
		return error_key("cannot read key");
	}
	int keysym = key->f.keysym;

	if (keysym < KEYSYM_BASE) {
		Key k = {
			.type = KEY_TYPE_CHAR,
			.data = {
				.c = (char) keysym,
			},
		};
		return k;
	} else {
		int idx = keysym - KEYSYM_BASE;
		char *name = g_ptr_array_index(kb->names, idx);
		g_assert(name != NULL);
		Key k = {
			.type = KEY_TYPE_NAME,
			.data = {
				.name = name,
			},
		};
		return k;
	}
}

inline static guint find_free_index(GPtrArray *a)
{
	for (int i = 0; i < a->len; i++) {
		if (g_ptr_array_index(a, i) == NULL)
			return i;
	}

	guint size = a->len;
	g_ptr_array_set_size(a, size + 1);
	return size;
}

inline static Key error_key(char *msg) {
	Key k = {
		.type = KEY_TYPE_ERROR,
		.data = {
			.error = msg,
		},
	};
	return k;
}


inline static void init_keymap(Keyboard *kb)
{
   	char esc_seq[10];

   	esc_seq[1] = 0;
   	for (int i = 1; i < 256; i++) {
   	   	esc_seq[0] = (char) i;
   	   	SLkm_define_keysym(esc_seq, i, kb->keymap);
   	}

   	/* Now add most common ones. */
   	/*
   	spoon_keyboard_defkey(kb, "^@", 0);
   	spoon_keyboard_defkey(kb, "\033[A", SL_KEY_UP);
   	spoon_keyboard_defkey(kb, "\033OA", SL_KEY_UP);
   	spoon_keyboard_defkey(kb, "\033[B", SL_KEY_DOWN);
   	spoon_keyboard_defkey(kb, "\033OB", SL_KEY_DOWN);
   	spoon_keyboard_defkey(kb, "\033[C", SL_KEY_RIGHT);
   	spoon_keyboard_defkey(kb, "\033OC", SL_KEY_RIGHT);
   	spoon_keyboard_defkey(kb, "\033[D", SL_KEY_LEFT);
   	spoon_keyboard_defkey(kb, "\033OD", SL_KEY_LEFT);
   	spoon_keyboard_defkey(kb, "\033[F", SL_KEY_END);
   	spoon_keyboard_defkey(kb, "\033OF", SL_KEY_END);
   	spoon_keyboard_defkey(kb, "\033[H", SL_KEY_HOME);
   	spoon_keyboard_defkey(kb, "\033OH", SL_KEY_HOME);
   	spoon_keyboard_defkey(kb, "\033[2~", SL_KEY_IC);
   	spoon_keyboard_defkey(kb, "\033[3~", SL_KEY_DELETE);
   	spoon_keyboard_defkey(kb, "\033[5~", SL_KEY_PPAGE);
   	spoon_keyboard_defkey(kb, "\033[6~", SL_KEY_NPAGE);
   	spoon_keyboard_defkey(kb, "\033[7~", SL_KEY_HOME);
   	spoon_keyboard_defkey(kb, "\033[8~", SL_KEY_END);
   	*/

   	strcpy (esc_seq, "^(kX)");
   	for (int i = 0; i <= 9; i++)
   	{
   		GString *string = g_string_new("");
   		g_string_printf(string, "F%d", i);
	   	esc_seq[3] = '0' + i;
	   	spoon_keyboard_defkey(kb, esc_seq, string->str);
	   	g_string_free(string, TRUE);
   	}
   	spoon_keyboard_defkey(kb, "^(k;)", "F10");
   	spoon_keyboard_defkey(kb, "^(F1)", "F11");
   	spoon_keyboard_defkey(kb, "^(F2)", "F12");

   	spoon_keyboard_defkey(kb, "^(ku)", "UP");
   	spoon_keyboard_defkey(kb, "^(kd)", "DOWN");
   	spoon_keyboard_defkey(kb, "^(kl)", "LEFT");
   	spoon_keyboard_defkey(kb, "^(kr)", "RIGHT");
   	spoon_keyboard_defkey(kb, "^(kP)", "PPAGE");
   	spoon_keyboard_defkey(kb, "^(kN)", "NPAGE");
   	spoon_keyboard_defkey(kb, "^(kh)", "HOME");
   	spoon_keyboard_defkey(kb, "^(@7)", "END");
   	spoon_keyboard_defkey(kb, "^(K1)", "A1");
   	spoon_keyboard_defkey(kb, "^(K3)", "A3");
   	spoon_keyboard_defkey(kb, "^(K2)", "B2");
   	spoon_keyboard_defkey(kb, "^(K4)", "C1");
   	spoon_keyboard_defkey(kb, "^(K5)", "C3");
   	spoon_keyboard_defkey(kb, "^(%0)", "REDO");
   	spoon_keyboard_defkey(kb, "^(&8)", "UNDO");
   	spoon_keyboard_defkey(kb, "^(kb)", "BACKSPACE");
   	spoon_keyboard_defkey(kb, "^(@8)", "ENTER");
   	spoon_keyboard_defkey(kb, "^(kD)", "DELETE");
}

