/* vim: set ft=c.doxygen noexpandtab ts=4 sw=4 tw=80: */
#ifndef __MOONSHINE_NET_H__
#define __MOONSHINE_NET_H__
#include <glib.h>

/** \file net.h
 * \brief Async TCP/IP socket connection library.
 *
 * Ready the net module for connection requests.
 * (Note, this requires the glib mainloop to be running).
 *
 * \code
 * net_start()
 * \endcode
 *
 * Initiate a connection to example.com on port 8080.
 *
 * \code
 * net_connect("localhost", "8080", callback, NULL);
 * \endcode
 *
 * To handle the connection, the callback function needs to be defined.
 * For details on the callback function, see \ref NetFunc and \ref NetEvent.
 *
 * \example test-net.c
 * Example usage of this module.
 *
 */


/** Type of NetEvent.
 * Currently there are three types of events: connections, dns errors, and
 * system errors.
 */
typedef enum {
	/// Sucessfuly connected.
	NET_CONNECT,
	/// Error during name resolution (see getaddrinfo).
	NET_ERROR,
	/// Error during connect.
	NET_SYS_ERROR
} NetEventType;

/** This structure represents a connection event.
 * A connection event is either a failure or a success.
 * Successful events always have the type \ref NET_CONNECT.
 * Failures have either the type \ref NET_ERROR or \ref NET_SYS_ERROR.
 *
 * For \ref NET_CONNECT events, the \ref fd field will point to a valid file descriptor.
 *
 * For \ref NET_ERROR events, the \ref error field contains the result of getaddrinfo().
 * Additionally, \ref sys_error will contain the value of errno (at the time of
 * error).
 *
 * For \ref NET_SYS_ERROR events, the \ref sys_error field will contain the value of errno
 * (at the time of error).
 */
typedef struct {
	/// The event type.
	NetEventType type;
	/// File descriptor.
	int fd;
	/// The return value of the POSIX function getaddrinfo().
	int error;
	/// The value of POSIX global errno at time of error.
	int sys_error;
} NetEvent;

/** Callback function type for \ref net_connect().
 *
 * This function is called in response to a \ref net_connect() call, at some
 * undetermined time.
 *
 * \param event The reason why this function was called.
 * \param data  The data parameter passed to \ref net_connect().
 */
typedef void (*NetFunc)(NetEvent event, gpointer data);

/** This function must be called before net_connect(). It establishes the
 * underlying thread pool and event queue. */
void net_start(void);

/** Attempt to asynchronously connect to a host using TCP/IP.
 *
 * This function schedules the DNS lookup and connection to a host.
 * 
 * \param hostname The hostname or IP address of the server.
 * \param service  The service name or port number.
 * \param func     The callback function to call on success or failure.
 * \param data     Extra information to pass to the above callback.
 */
void net_connect(const char *hostname, const char *service, NetFunc func,
		gpointer data);

void net_stop(void);
