/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "util.h"

PURE int unicode_charwidth(gunichar ch) {
	if (g_unichar_iswide(ch) || g_unichar_iscntrl(ch))
		return 2;
	else
		return 1;
}
