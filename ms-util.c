/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine/ms-util.h"

PURE int ms_unicode_charwidth(gunichar ch) {
	if (g_unichar_iswide(ch) || g_unichar_iscntrl(ch))
		return 2;
	else
		return 1;
}
