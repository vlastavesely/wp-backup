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

#include <wp-backup/http-request.h>

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
