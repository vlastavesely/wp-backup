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

#ifndef __HTTP_REQUEST_H
#define __HTTP_REQUEST_H

struct http_request;

enum http_method {
	HTTP_METHOD_GET = 0,
	HTTP_METHOD_POST = 1
};

struct http_request *http_request_new(char *url);
void http_request_free(struct http_request *request);

void http_request_set_method(struct http_request *request,
			     enum http_method method);
void http_request_set_body(struct http_request *request, char *body);
void http_request_set_filename(struct http_request *request, char *filename);

enum http_method http_request_get_method(struct http_request *request);
char *http_request_get_url(struct http_request *request);
char *http_request_get_body(struct http_request *request);
char *http_request_get_filename(struct http_request *request);

#endif /* __HTTP_REQUEST_H */
