/* vim: set ft=c.doxygen noexpandtab ts=4 sw=4 tw=80: */
#ifndef __MOONSHINE_NET_H__
#define __MOONSHINE_NET_H__
#include <glib.h>

/** \file net.h
 * \brief Async TCP/IP socket connection library.
 *
 * Ready the net module for connection requests.
 * (Note, this requires the glib mainloop to be running).
 */


/** This GError domain represents errors from getaddrinfo() */
#define NET_ERROR_DNS g_quark_from_string("NetErrorDNS")

/** This GError domain represents errors from socket or connect. The code field is
 * errno value. */
#define NET_ERROR_SYS g_quark_from_string("NetErrorSys")

void net_start(void);

typedef void (*NetConnectFunc)(int fd, gpointer data);
typedef void (*NetErrorFunc)(GError *err, gpointer data);
void net_connect(const char *hostname, const char *service,
		NetConnectFunc on_connect,
		NetErrorFunc on_error,
		gpointer data);

void net_stop(void);


#endif
