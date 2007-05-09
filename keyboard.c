#include "config.h"
#include "keyboard.h"


const int KEYSYM_BASE = 0x256;

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

Keyboard *spoon_keyboard_new(void)
{
	char s[] = {0, 0, 0};
	Keyboard *kb = g_new(Keyboard, 1);
	kb->names    = g_ptr_array_new();
	kb->keymap   = SLang_create_keymap("default", NULL);

	for (int i = 0; i < 256; i++) {
		s[0] = i;
		SLkm_define_keysym(g_strdup(s), i, kb->keymap);
	}

	return kb;
}


void spoon_keyboard_define(Keyboard *kb, char *spec, char *name)
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


static inline init_keymap(Keyboard *kb)
{
   char esc_seq[10];
   int i;

   esc_seq[1] = 0;
   for (i = 1; i < 256; i++)
     {
	esc_seq[0] = (char) i;
	SLkm_define_keysym (esc_seq, i);
     }

   /* Now add most common ones. */
#ifndef IBMPC_SYSTEM
   spoon_keyboard_define(kb, "^@", 0);

   spoon_keyboard_define(kb, "\033[A", SL_KEY_UP);
   spoon_keyboard_define(kb, "\033OA", SL_KEY_UP);
   spoon_keyboard_define(kb, "\033[B", SL_KEY_DOWN);
   spoon_keyboard_define(kb, "\033OB", SL_KEY_DOWN);
   spoon_keyboard_define(kb, "\033[C", SL_KEY_RIGHT);
   spoon_keyboard_define(kb, "\033OC", SL_KEY_RIGHT);
   spoon_keyboard_define(kb, "\033[D", SL_KEY_LEFT);
   spoon_keyboard_define(kb, "\033OD", SL_KEY_LEFT);
   spoon_keyboard_define(kb, "\033[F", SL_KEY_END);
   spoon_keyboard_define(kb, "\033OF", SL_KEY_END);
   spoon_keyboard_define(kb, "\033[H", SL_KEY_HOME);
   spoon_keyboard_define(kb, "\033OH", SL_KEY_HOME);
   spoon_keyboard_define(kb, "\033[2~", SL_KEY_IC);
   spoon_keyboard_define(kb, "\033[3~", SL_KEY_DELETE);
   spoon_keyboard_define(kb, "\033[5~", SL_KEY_PPAGE);
   spoon_keyboard_define(kb, "\033[6~", SL_KEY_NPAGE);
   spoon_keyboard_define(kb, "\033[7~", SL_KEY_HOME);
   spoon_keyboard_define(kb, "\033[8~", SL_KEY_END);
#else
   /* Note: This will not work if SLgetkey_map_to_ansi (1) has
    * been called.
    */
   spoon_keyboard_define(kb, "^@\x48", SL_KEY_UP );
   spoon_keyboard_define(kb, "^@\x50", SL_KEY_DOWN );
   spoon_keyboard_define(kb, "^@\x4d", SL_KEY_RIGHT );
   spoon_keyboard_define(kb, "^@\x4b", SL_KEY_LEFT );
   spoon_keyboard_define(kb, "^@\x47", SL_KEY_HOME );
   spoon_keyboard_define(kb, "^@\x49", SL_KEY_PPAGE );
   spoon_keyboard_define(kb, "^@\x51", SL_KEY_NPAGE );
   spoon_keyboard_define(kb, "^@\x4f", SL_KEY_END );
   spoon_keyboard_define(kb, "^@\x52", SL_KEY_IC );
   spoon_keyboard_define(kb, "^@\x53", SL_KEY_DELETE );
   
   spoon_keyboard_define(kb, "\xE0\x48", SL_KEY_UP );
   spoon_keyboard_define(kb, "\xE0\x50", SL_KEY_DOWN );
   spoon_keyboard_define(kb, "\xE0\x4d", SL_KEY_RIGHT );
   spoon_keyboard_define(kb, "\xE0\x4b", SL_KEY_LEFT );
   spoon_keyboard_define(kb, "\xE0\x47", SL_KEY_HOME );
   spoon_keyboard_define(kb, "\xE0\x49", SL_KEY_PPAGE );
   spoon_keyboard_define(kb, "\xE0\x51", SL_KEY_NPAGE );
   spoon_keyboard_define(kb, "\xE0\x4f", SL_KEY_END );
   spoon_keyboard_define(kb, "\xE0\x52", SL_KEY_IC );
   spoon_keyboard_define(kb, "\xE0\x53", SL_KEY_DELETE );

    strcpy (esc_seq, "^@ ");	       /* guarantees esc_seq[3] = 0. */

    for (i = 0x3b; i < 0x45; i++)
      {
	 esc_seq [2] = i;
	 spoon_keyboard_define(kb, esc_seq, SL_KEY_F(i - 0x3a));
      }
   esc_seq[2] = 0x57; spoon_keyboard_define(kb, esc_seq, SL_KEY_F(11));
   esc_seq[2] = 0x58; spoon_keyboard_define(kb, esc_seq, SL_KEY_F(12));
#endif

#ifdef REAL_UNIX_SYSTEM
   strcpy (esc_seq, "^(kX)");
   for (i = 0; i <= 9; i++)
   {
	   esc_seq[3] = '0' + i;
	   spoon_keyboard_define(kb, esc_seq, SL_KEY_F(i));
   }
   spoon_keyboard_define(kb, "^(k;)", SL_KEY_F(10));
   spoon_keyboard_define(kb, "^(F1)", SL_KEY_F(11));
   spoon_keyboard_define(kb, "^(F2)", SL_KEY_F(12));

   spoon_keyboard_define(kb, "^(ku)", SL_KEY_UP);
   spoon_keyboard_define(kb, "^(kd)", SL_KEY_DOWN);
   spoon_keyboard_define(kb, "^(kl)", SL_KEY_LEFT);
   spoon_keyboard_define(kb, "^(kr)", SL_KEY_RIGHT);
   spoon_keyboard_define(kb, "^(kP)", SL_KEY_PPAGE);
   spoon_keyboard_define(kb, "^(kN)", SL_KEY_NPAGE);
   spoon_keyboard_define(kb, "^(kh)", SL_KEY_HOME);
   spoon_keyboard_define(kb, "^(@7)", SL_KEY_END);
   spoon_keyboard_define(kb, "^(K1)", SL_KEY_A1);
   spoon_keyboard_define(kb, "^(K3)", SL_KEY_A3);
   spoon_keyboard_define(kb, "^(K2)", SL_KEY_B2);
   spoon_keyboard_define(kb, "^(K4)", SL_KEY_C1);
   spoon_keyboard_define(kb, "^(K5)", SL_KEY_C3);
   spoon_keyboard_define(kb, "^(%0)", SL_KEY_REDO);
   spoon_keyboard_define(kb, "^(&8)", SL_KEY_UNDO);
   spoon_keyboard_define(kb, "^(kb)", SL_KEY_BACKSPACE);
   spoon_keyboard_define(kb, "^(@8)", SL_KEY_ENTER);
   spoon_keyboard_define(kb, "^(kD)", SL_KEY_DELETE);
#endif
}


