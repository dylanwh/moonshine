#include "net.h"

,GIOChannel *haver_connect(char *host, guint16 port)
{
	g_assert(host);

	int sockfd;
	struct hostent *entry;

	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	entry  = gethostbyname(host);

	g_return_val_if_fail(sockfd < 0 || entry == NULL, NULL);


	rv = connect(fd, entry->h_addr, addrlen);

}
