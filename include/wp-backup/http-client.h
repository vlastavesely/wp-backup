#ifndef __HTTP_CLIENT_H
#define __HTTP_CLIENT_H

#include <wp-backup/http-request.h>
#include <wp-backup/http-response.h>

struct http_client;

struct http_client *http_client_new();

void http_client_free(struct http_client *client);


struct http_response *http_client_send(struct http_client *client,
				       struct http_request *request);

#endif /* __HTTP_CLIENT_H */
