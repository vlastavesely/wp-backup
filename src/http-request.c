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

	request->method = HTTP_METHOD_GET;
	request->url = strdup(url);
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
	request->body = strdup(body);
	request->content_type = strdup(content_type);
}


enum http_method http_request_get_method(struct http_request *request)
{
	return request->method;
}

char *http_request_get_url(struct http_request *request)
{
	return strdup(request->url);
}

char *http_request_get_content_type(struct http_request *request)
{
	if (request->content_type) {
		return strdup(request->content_type);
	} else {
		return NULL;
	}
}

char *http_request_get_body(struct http_request *request)
{
	if (request->body) {
		return strdup(request->body);
	} else {
		return NULL;
	}
}
