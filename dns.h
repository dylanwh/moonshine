#ifndef DNS_H
#define DNS_H 1

#include <netdb.h>

typedef void (*DNSCallback)(struct hostent *result, int errno, void *userdata);
void dnsrequest(const char *name, DNSCallback callback, void *userdata);

#endif

