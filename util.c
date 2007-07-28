/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#include "moonshine.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>


PURE int unicode_charwidth(gunichar ch) {
	if (g_unichar_iswide(ch))
		return 2;
	else
		return 1;
}
