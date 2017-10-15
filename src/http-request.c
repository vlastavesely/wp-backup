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
	char *body;
	char *filename;
};

struct http_request *http_request_new(char *url)
{
	struct http_request *request = malloc(sizeof(*request));

	request->method = HTTP_METHOD_GET;
	request->url = strdup(url);
	request->body = NULL;
	request->filename = NULL;
	return request;
}

void http_request_free(struct http_request *request)
{
	free(request->url);
	if (request->body) {
		/* May contain sensitive data like passwords, etc. */
		memset(request->body, 0, strlen(request->body));
		free(request->body);
	}
	if (request->filename)
		free(request->filename);
	free(request);
}

void http_request_set_method(struct http_request *request,
			     enum http_method method)
{
	request->method = method;
}

void http_request_set_body(struct http_request *request, char *body)
{
	request->body = strdup(body);
}

void http_request_set_filename(struct http_request *request, char *filename)
{
	request->filename = strdup(filename);
}

enum http_method http_request_get_method(struct http_request *request)
{
	return request->method;
}

char *http_request_get_url(struct http_request *request)
{
	return strdup(request->url);
}

char *http_request_get_body(struct http_request *request)
{
	return (request->body) ? strdup(request->body) : NULL;
}

char *http_request_get_filename(struct http_request *request)
{
	return (request->filename) ? strdup(request->filename) : NULL;
}
