#ifndef __HTTP_RESPONSE_H
#define __HTTP_RESPONSE_H

struct http_response;

struct http_response *http_response_new(int code, char *body);

void http_response_free(struct http_response *response);


int http_response_get_code(struct http_response *response);

char *http_response_get_body(struct http_response *response);

#endif /* __HTTP_RESPONSE_H */
