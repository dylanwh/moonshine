#include "dns.h"
#include <glib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>


typedef struct dnsreq {
    /* On buffer overflow, we immediately invoke callback with ERANGE */
    char dns_name[256];
    struct hostent *he_p;
    int read_end;
    int write_end;
    int errno_v;

    GIOChannel *read_chan;
    guint read_ev_id;
    pthread_t worker;
    DNSCallback callback;
    void *userdata;

    struct hostent he;
    void *auxdata;
} dnsreq_t;

#ifndef HAVE_GETHOSTBYNAME_R
static pthread_mutex_t dnsmutex = PTHREAD_MUTEX_INITIALIZER;

static void clone_hostent(struct hostent * restrict dest, void * restrict *auxdata, const struct hostent * restrict src) {
    int alias_count = 0;
    int addr_count  = 0;
    size_t total_sz = strlen(src->h_name) + 1;
    char *allocp;

    dest->h_addrtype = src->h_addrtype;
    dest->h_length = src->h_length;
    for (char **h_aliasp = src->h_aliases; *h_aliasp; h_aliasp++) {
        total_sz += strlen(*h_aliasp) + 1;
        alias_count++;
    }
    total_sz += (alias_count + 1) * sizeof(char *);
    for (char **h_addrp = src->h_addr_list; *h_addrp; h_addrp++)
        addr_count++;
    total_sz += (addr_count + 1) * sizeof(char *);
    total_sz += addr_count * src->h_length;
    allocp = *auxdata = g_malloc(total_sz);

    dest->h_aliases = (char **)allocp;
    allocp += (alias_count + 1) * sizeof(char *);
    dest->h_addr_list = (char **)allocp;
    allocp += (addr_count + 1) * sizeof(char *);

    for (int i = 0; i < addr_count; i++) {
        dest->h_addr_list[i] = allocp;
        allocp += src->h_length;
        memcpy(dest->h_addr_list[i], src->h_addr_list[i], src->h_length);
    }
    dest->h_addr_list[addr_count] = NULL;

    dest->h_name = (char *)allocp;
    allocp += strlen(src->h_name) + 1;
    strcpy(dest->h_name, src->h_name);

    for (int i = 0; i < alias_count; i++) {
        dest->h_aliases[i] = (char *)allocp;
        allocp += strlen(src->h_aliases[i]) + 1;
        strcpy(dest->h_aliases[i], src->h_aliases[i]);
    }
    dest->h_aliases[alias_count] = NULL;
    assert(allocp - total_sz == *auxdata);
}

#endif

static void do_lookup(dnsreq_t *dr) {
    extern int h_errno;
#ifndef HAVE_GETHOSTBYNAME_R
    pthread_mutex_lock(&dnsmutex);
    dr->he_p = gethostbyname(dr->dns_name);
    if (!dr->he_p) {
        dr->errno_v = h_errno;
    } else {
        clone_hostent(&dr->he, &dr->auxdata, dr->he_p);
        dr->he_p = &dr->he;
    }
    pthread_mutex_unlock(&dnsmutex);
#else
    size_t bufsz = 256;
    const size_t bufmax = 4096;
    int h_errno, retval;
try_lookup:
    g_free(dr->auxdata); // init is NULL so this is safe
    dr->auxdata = g_malloc(bufsz);
    retval = gethostbyname_r(dr->dns_name, &dr->he, dr->auxdata, bufsz, &dr->he_p, &h_errno);
    if (retval == ERANGE) {
        if (bufsz < bufmax) {
            bufsz *= 2;
            if (bufsz > bufmax)
                bufsz = bufmax;
            goto try_lookup;
        }
        h_errno = ERANGE;
    }
    if (retval != 0 || !dr->he_p) {
        g_free(dr->auxdata);
        dr->auxdata = NULL;
        dr->he_p = NULL;
        dr->errno_v = retval ? retval : h_errno;
        return;
    } 
    dr->errno_v = 0;
#endif
}

static gboolean complete_cb(GIOChannel *source, GIOCondition condition, gpointer dr_vp) {
    dnsreq_t *dr = dr_vp;
    GError *dummy = NULL;

    int ret = pthread_join(dr->worker, NULL);
    if (ret) {
        fprintf(stderr, "pthread_join: %s\n", strerror(ret));
        abort();
    }
    
    if (dr->he_p)
        dr->callback(dr->he_p, 0, dr->userdata);
    else
        dr->callback(NULL, dr->errno_v, dr->userdata);
    g_free(dr->auxdata);
    g_source_remove(dr->read_ev_id);
    g_io_channel_shutdown(dr->read_chan, FALSE, &dummy);
    if (dummy)
        g_error_free(dummy);
    close(dr->read_end);
    close(dr->write_end);
    g_free(dr);
    return FALSE;
}

static void *worker(void *dr_vp) {
    printf("worker start\n");
    dnsreq_t *dr = dr_vp;
    do_lookup(dr);
    write(dr->write_end, dr, sizeof dr);
    printf("worker done\n");
    return NULL;
}

void dnsrequest(const char *name, DNSCallback callback, void *userdata) {
    pthread_attr_t attr;
    if (pthread_attr_init(&attr)) {
        callback(NULL, errno, userdata);
        return;
    }

    dnsreq_t *dr = g_malloc(sizeof *dr);
    int sv[2];

    dr->he_p    = (struct hostent *)0xFEEDBEEF;
    dr->auxdata = NULL;

    dr->read_end = -1;
    dr->write_end = -1;
    dr->read_chan = NULL;
    dr->callback = callback;
    dr->userdata = userdata;

    if (strlen(name) > sizeof dr->dns_name - 1) {
        errno = ERANGE;
        goto err_out;
    }

    strcpy(dr->dns_name, name);
    if (pipe(sv))
        goto err_out;
    
    dr->read_end = sv[0];
    dr->write_end = sv[1];
    dr->read_chan = g_io_channel_unix_new(dr->read_end);
    if (!dr->read_chan)
        goto err_out;
    dr->read_ev_id = g_io_add_watch(dr->read_chan, G_IO_IN, complete_cb, dr);
    
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (pthread_create(&dr->worker, NULL, worker, dr))
        goto err_out;
    return;
err_out:
    assert(!dr->worker);
    if (dr->read_chan) {
        GError *dummy = NULL;
        g_source_remove(dr->read_ev_id);
        g_io_channel_shutdown(dr->read_chan, FALSE, &dummy);
        if (dummy)
            g_error_free(dummy);
    } else {
        if (dr->read_end != -1)
            close(dr->read_end);
    }
    if (dr->write_end != -1)
        close(dr->write_end);

    callback(NULL, errno, userdata);
    pthread_attr_destroy(&attr);
}

#ifdef DNS_TEST
static int dnsc = 0;
static void callback(struct hostent *result, int herrno, void *userdata) {
    GMainLoop *loop = userdata;
    if (result) {
        printf("Lookup successful.\n");
        printf("h_name = \"%s\"\n", result->h_name);
        printf("h_aliases = {\n");
        for (char **p = result->h_aliases; *p; p++)
            printf("\t\"%s\",\n", *p);
        printf("\tNULL\n}\n");
        printf("h_addrtype = %d\n", result->h_addrtype);
        printf("h_length = %d\n", result->h_length);
        printf("h_addr_list = {\n");
        for (char **p = result->h_addr_list; *p; p++) {
            char tmpbuf[256];
            if (inet_ntop(result->h_addrtype, *p, tmpbuf, sizeof tmpbuf)) {
                printf("\t{ %s },\n", tmpbuf);
            } else {
                printf("\t{ ??? },\n");
            }
        }
        printf("\tNULL\n}\n");
    } else {
        printf("Lookup error: %s\n", strerror(herrno));
    }
    if (!--dnsc)
        g_main_loop_quit(loop);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s hostname [...]\n", argc ? argv[0] : "dnstest");
        return 1;
    }
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    for (int i = 1; i < argc; i++) {
        printf("Starting async lookup of %s...\n", argv[i]);
        dnsrequest(argv[i], callback, loop);   
        dnsc++;
    }
    printf("Lookup running, starting glib mainloop.\n");
    g_main_loop_run(loop);
    printf("Mainloop terminated.\n");
    g_main_loop_unref(loop);
    return 0;

}
#endif
