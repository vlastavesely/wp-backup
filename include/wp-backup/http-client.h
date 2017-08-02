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

#ifndef __HTTP_CLIENT_H
#define __HTTP_CLIENT_H

#include <wp-backup/http-request.h>
#include <wp-backup/http-response.h>

struct http_client;

struct http_client *http_client_new();

void http_client_free(struct http_client *client);


struct http_response *http_client_send(struct http_client *client,
				       struct http_request *request);

void http_client_skip_ssl_validation(struct http_client *client);

#endif /* __HTTP_CLIENT_H */
