/* vim: set ft=c noexpandtab ts=4 sw=4 tw=80 */
#ifndef __SPOON_PROTOCOL_H__
#define __SPOON_PROTOCOL_H__
#include <glib.h>
#include "config.h"

#define HAVER_NO_ERROR			0 ///< Decode was successful
#define HAVER_UNKNOWN_ESCAPE	1 ///< Unknown escape code in input
#define HAVER_TRUNCATED_ESCAPE	2 ///< Truncated escape (eg, ESC NL or ESC TAB)
#define HAVER_TRUNCATED_INPUT	3 ///< No newline at end of input
#define HAVER_BAD_UTF8			4 ///< Input is invalid unicode
#define HAVER_BAD_CHARS			5 ///< Input contains illegal characters
#define HAVER_EMPTY_ARR			6 ///< An array of zero elements was passed to encode

/** \brief Returns a human-readable form of a haver error code
 *
 * \param err The error code in question
 * \return A new GString containing a human-readable error message
 */
GString *haver_strerror(int err);

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
GPtrArray *haver_decode(const gchar *line, const gchar **remain, int *error);

/** \brief Encodes a haver protocol message into a network-ready format.
 *
 * \param line A GPtrArray of GStrings in UTF-8 coding.
 * \param error If not NULL, a pointer to a location in which to store the
 *   error code.
 * \return A GString in network-ready format if successful, or NULL otherwise.
 */
GString *haver_encode(const GPtrArray *line, int *error);

#endif

