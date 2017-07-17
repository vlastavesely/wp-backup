#ifndef __WORDPRESS_CONNECTION_H
#define __WORDPRESS_CONNECTION_H

#include <stdio.h>
#include <stdbool.h>

struct wordpress_connection;


struct wordpress_connection *wordpress_connection_initialize(const char *wpurl);

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
