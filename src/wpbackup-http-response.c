#include <stdlib.h>
#include <string.h>

#include <wpbackup-http-response.h>

struct wpbackup_http_response
{
	int code;
	char *body;
};

struct wpbackup_http_response *wpbackup_http_response_new(int code, const char *body)
{
	struct wpbackup_http_response *response
		= malloc(sizeof(struct wpbackup_http_response));

	response->code = code;
	response->body = malloc(strlen(body) + 1);
	strcpy(response->body, body);
	return response;
}

void wpbackup_http_response_free(struct wpbackup_http_response *response)
{
	if (response->body) {
		free(response->body);
	}
	free(response);
}


int wpbackup_http_response_get_code(struct wpbackup_http_response *response)
{
	return response->code;
}

char *wpbackup_http_response_get_body(struct wpbackup_http_response *response)
{
	char *s = malloc(strlen(response->body) + 1);
	strcpy(s, response->body);
	return s;
}
