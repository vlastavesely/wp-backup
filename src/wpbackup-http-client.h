#ifndef __WPBACKUP_HTTP_CLIENT_H
#define __WPBACKUP_HTTP_CLIENT_H

#include <wpbackup.h>

struct wpbackup_http_client;

struct wpbackup_http_client *wpbackup_http_client_new();

void wpbackup_http_client_free(struct wpbackup_http_client *client);


struct wpbackup_http_response *
wpbackup_http_client_send(struct wpbackup_http_client *client,
			  struct wpbackup_http_request *request);

#endif /* __WPBACKUP_HTTP_CLIENT_H */
