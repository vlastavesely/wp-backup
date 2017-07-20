#include <stdlib.h>
#include <string.h>

#include <wp-backup/http-response.h>

struct http_response
{
	int code;
	char *body;
	size_t length;
};

struct http_response *http_response_new(int code, unsigned char *body,
					size_t length)
{
	struct http_response *response
		= malloc(sizeof(struct http_response));

	response->code = code;

	response->body = malloc(length);
	memcpy(response->body, body, length);
	response->length = length;
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

unsigned char *http_response_get_body(struct http_response *response)
{
	unsigned char *body = malloc(response->length);
	memcpy(body, response->body, response->length);
	return body;
}

size_t http_response_get_body_length(struct http_response *response)
{
	return response->length;
}
