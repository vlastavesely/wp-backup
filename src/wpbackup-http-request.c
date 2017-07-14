#include <stdlib.h>
#include <string.h>

#include <wpbackup-http-request.h>

struct wpbackup_http_request
{
	char *url;
	char *content_type;
	char *body;
};

struct wpbackup_http_request *wpbackup_http_request_new(char *url)
{
	struct wpbackup_http_request *request
		= malloc(sizeof(struct wpbackup_http_request));

	request->url = malloc(strlen(url) + 1);
	strcpy(request->url, url);
	request->content_type = NULL;
	request->body = NULL;
	return request;
}

void wpbackup_http_request_free(struct wpbackup_http_request *request)
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


void wpbackup_http_request_set_body(struct wpbackup_http_request *request,
				    char *body, char *content_type)
{
	request->body = malloc(strlen(body) + 1);
	strcpy(request->body, body);
	request->content_type = malloc(strlen(content_type) + 1);
	strcpy(request->content_type, content_type);
}


char *wpbackup_http_request_get_url(struct wpbackup_http_request *request)
{
	char *s = malloc(strlen(request->url) + 1);
	strcpy(s, request->url);
	return s;
}

char *wpbackup_http_request_get_content_type(struct wpbackup_http_request *request)
{
	char *s = NULL;
	if (request->content_type) {
		s = malloc(strlen(request->content_type) + 1);
		strcpy(s, request->content_type);
	}
	return s;
}

char *wpbackup_http_request_get_body(struct wpbackup_http_request *request)
{
	char *s = NULL;
	if (request->body) {
		s = malloc(strlen(request->body) + 1);
		strcpy(s, request->body);
	}
	return s;
}
