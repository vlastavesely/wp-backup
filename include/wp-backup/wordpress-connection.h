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

#ifndef __WORDPRESS_CONNECTION_H
#define __WORDPRESS_CONNECTION_H

#include <stdio.h>
#include <stdbool.h>
#include <wp-backup/options.h>

struct wordpress_connection;


struct wordpress_connection *
wordpress_connection_initialize(struct options *options);

void wordpress_connection_free(struct wordpress_connection *connection);


bool wordpress_connection_login(struct wordpress_connection *connection,
				const char *username, const char *password);

bool wordpress_connection_logout(struct wordpress_connection *connection);

void
wordpress_connection_download_to_stream(struct wordpress_connection *connection,
					char *url, FILE *stream);

void
wordpress_connection_download_to_file(struct wordpress_connection *connection,
				      char *url, const char *filename);

#endif /* __WORDPRESS_CONNECTION_H */
