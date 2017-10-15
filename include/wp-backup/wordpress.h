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

#ifndef __WORDPRESS_H
#define __WORDPRESS_H

#include <stdio.h>
#include <stdbool.h>
#include <wp-backup/options.h>

struct wordpress;


struct wordpress *wordpress_create(const char *wpurl);

void wordpress_free(struct wordpress *connection);


int wordpress_login(struct wordpress *connection,
		    const char *username, const char *password);

bool wordpress_logout(struct wordpress *connection);

int wordpress_export(struct wordpress *connection,
		     const char *filename);

int wordpress_download_to_file(struct wordpress *connection, char *url,
			       const char *filename);

#endif /* __WORDPRESS_H */
