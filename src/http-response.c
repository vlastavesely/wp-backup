#include <stdlib.h>
#include <string.h>

#include <wpbackup/http-response.h>

struct http_response
{
	int code;
	char *body;
};

struct http_response *http_response_new(int code, char *body)
{
	struct http_response *response
		= malloc(sizeof(struct http_response));

	response->code = code;
	response->body = malloc(strlen(body) + 1);
	strcpy(response->body, body);
	return response;
}

void http_response_free(struct http_response *response)
{
	if (response->body) {
		free(response->body);
	}
	free(response);
}


int http_response_get_code(struct http_response *response)
{
	return response->code;
}

char *http_response_get_body(struct http_response *response)
{
	char *s = malloc(strlen(response->body) + 1);
	strcpy(s, response->body);
	return s;
}