#ifndef __HTTP_REQUEST_H
#define __HTTP_REQUEST_H

struct http_request;

enum http_method
{
	HTTP_METHOD_GET = 0,
	HTTP_METHOD_POST = 1
};

struct http_request *http_request_new(char *url);

void http_request_free(struct http_request *request);


void http_request_set_method(struct http_request *request,
			     enum http_method method);

void http_request_set_body(struct http_request *request,
			   char *body, char *content_type);


enum http_method http_request_get_method(struct http_request *request);

char *http_request_get_url(struct http_request *request);

char *http_request_get_content_type(struct http_request *request);

char *http_request_get_body(struct http_request *request);

#endif /* __HTTP_REQUEST_H */
