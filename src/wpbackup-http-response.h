#ifndef __WPBACKUP_HTTP_RESPONSE_H
#define __WPBACKUP_HTTP_RESPONSE_H

struct wpbackup_http_response;

struct wpbackup_http_response *wpbackup_http_response_new(int code, char *body);

void wpbackup_http_response_free(struct wpbackup_http_response *response);


int wpbackup_http_response_get_code(struct wpbackup_http_response *response);

char *wpbackup_http_response_get_body(struct wpbackup_http_response *response);

#endif /* __WPBACKUP_HTTP_RESPONSE_H */
