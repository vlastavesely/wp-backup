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

#ifndef __HTTP_H
#define __HTTP_H

struct http_request {
	const char *method;
	const char *url;
	const char *body;
};

struct http_response {
	int code;
	char *body;
	size_t length;
	char *content_type;
};

struct http_client *http_client_alloc(void);
void http_client_drop(struct http_client *client);

void http_response_drop(struct http_response *response);

struct http_response *http_client_send(struct http_client *client,
		struct http_request *request);

int http_client_download_file(struct http_client *client,
		struct http_request *request,
		const char *filename);

#endif /* __HTTP_H */
