/* vim: set ft=c.doxygen noexpandtab ts=4 sw=4 tw=80 */
#ifndef __MOONSHINE_PROTOCOL_H__
#define __MOONSHINE_PROTOCOL_H__

#include <glib.h>
#include "config.h"

typedef enum {
	HAVER_NO_ERROR,
	HAVER_UNKNOWN_ESCAPE,
	HAVER_TRUNCATED_ESCAPE,
	HAVER_TRUNCATED_INPUT,
	HAVER_BAD_UTF8,
	HAVER_BAD_CHARS,
	HAVER_EMPTY_ARR,
} HaverError;

/** \brief Returns a human-readable form of a haver error code
 *
 * \param err The error code in question
 * \return A new GString containing a human-readable error message
 */
GString *haver_strerror(HaverError err);

/** \brief Breaks a haver protocol message into its tab-delimited items,
 *   replacing escape sequences.
 *
 * \param line The null-delimited string to decode
 * \param remain If not NULL, a pointer to a location in which to store the
 *   location of beginning of the next line (or the null-delimiter, if there
 *   is no next line)
 * \param error If not NULL, a pointer to a location in which to store the error
 *   code.
 * \return A GPtrArray of GStrings in UTF8 coding, if successful, or NULL
 *   otherwise.
 */
GPtrArray *haver_decode(const gchar *line, const gchar **remain, HaverError *error);

/** \brief Encodes a haver protocol message into a network-ready format.
 *
 * \param line A GPtrArray of GStrings in UTF-8 coding.
 * \param error If not NULL, a pointer to a location in which to store the
 *   error code.
 * \return A GString in network-ready format if successful, or NULL otherwise.
 */
GString *haver_encode(const GPtrArray *line, HaverError *error);

#endif

