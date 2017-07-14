#ifndef __WPBACKUP_HTTP_REQUEST_H
#define __WPBACKUP_HTTP_REQUEST_H

struct wpbackup_http_request;

struct wpbackup_http_request *wpbackup_http_request_new(char *url);

void wpbackup_http_request_free(struct wpbackup_http_request *request);


void wpbackup_http_request_set_body(struct wpbackup_http_request *request,
				    char *body, char *content_type);


char *wpbackup_http_request_get_url(struct wpbackup_http_request *request);

char *wpbackup_http_request_get_content_type(struct wpbackup_http_request *request);

char *wpbackup_http_request_get_body(struct wpbackup_http_request *request);

#endif /* __WPBACKUP_HTTP_REQUEST_H */
