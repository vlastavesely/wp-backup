/**
 * Copyright (c) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include <wp-backup/http-response.h>

struct http_response {
	int code;
	char *body;
	size_t length;
};

struct http_response *http_response_new(int code, unsigned char *body,
					size_t length)
{
	struct http_response *response = malloc(sizeof(*response));

	response->code = code;
	if (body) {
		response->body = malloc(length);
		memcpy(response->body, body, length);
	} else {
		response->body = NULL;
	}
	response->length = length;
	return response;
}

void http_response_free(struct http_response *response)
{
	if (response->body)
		free(response->body);
	free(response);
}


int http_response_get_code(struct http_response *response)
{
	return response->code;
}

unsigned char *http_response_get_body(struct http_response *response)
{
	unsigned char *body;

	if (!response->body)
		return NULL;

	body = malloc(response->length);
	memcpy(body, response->body, response->length);
	return body;
}

size_t http_response_get_body_length(struct http_response *response)
{
	return response->length;
}
