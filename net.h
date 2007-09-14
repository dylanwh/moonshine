#include <glib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef enum {
	NET_CONNECT,
	NET_ERROR,
	NET_SYS_ERROR
} NetEventType;

typedef struct {
	NetEventType type;
	int fd, error, sys_error;
} NetEvent;

typedef void (*NetFunc)(NetEvent event, gpointer data);

void net_start(void);
void net_connect(const char *hostname, const char *service, NetFunc func,
		gpointer data);

void net_stop(void);
