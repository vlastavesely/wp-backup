#ifndef __HTTP_RESPONSE_H
#define __HTTP_RESPONSE_H

struct http_response;

struct http_response *http_response_new(int code, unsigned char *body,
					size_t length);

void http_response_free(struct http_response *response);


int http_response_get_code(struct http_response *response);

unsigned char *http_response_get_body(struct http_response *response);

size_t http_response_get_body_length(struct http_response *response);

#endif /* __HTTP_RESPONSE_H */
