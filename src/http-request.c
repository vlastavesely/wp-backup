#include <stdlib.h>
#include <string.h>

#include <wpbackup/http-request.h>

struct http_request
{
	int method;
	char *url;
	char *content_type;
	char *body;
};

struct http_request *http_request_new(char *url)
{
	struct http_request *request = malloc(sizeof(struct http_request));

	request->method = HTTP_METHOD_POST;
	request->url = malloc(strlen(url) + 1);
	strcpy(request->url, url);
	request->content_type = NULL;
	request->body = NULL;
	return request;
}

void http_request_free(struct http_request *request)
{
	if (request->url) {
		free(request->url);
	}
	if (request->body) {
		/* May contain sensitive data like passwords, etc. */
		memset(request->body, 0, strlen(request->body));
		free(request->body);
	}
	if (request->content_type) {
		free(request->content_type);
	}
	free(request);
}


void http_request_set_method(struct http_request *request,
			     enum http_method method)
{
	request->method = method;
}

void http_request_set_body(struct http_request *request,
			   char *body, char *content_type)
{
	request->body = malloc(strlen(body) + 1);
	strcpy(request->body, body);
	request->content_type = malloc(strlen(content_type) + 1);
	strcpy(request->content_type, content_type);
}


enum http_method http_request_get_method(struct http_request *request)
{
	return request->method;
}

char *http_request_get_url(struct http_request *request)
{
	char *s = malloc(strlen(request->url) + 1);
	strcpy(s, request->url);
	return s;
}

char *http_request_get_content_type(struct http_request *request)
{
	char *s = NULL;
	if (request->content_type) {
		s = malloc(strlen(request->content_type) + 1);
		strcpy(s, request->content_type);
	}
	return s;
}

char *http_request_get_body(struct http_request *request)
{
	char *s = NULL;
	if (request->body) {
		s = malloc(strlen(request->body) + 1);
		strcpy(s, request->body);
	}
	return s;
}
