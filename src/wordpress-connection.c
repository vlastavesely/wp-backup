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

#include <wp-backup.h>

struct wordpress_connection
{
	struct http_client *http_client;
	char *wpurl;
	char *logout_url;
};

static char *wordpress_connection_build_login_url(const char *wpurl)
{
	char *login_url;

	login_url = malloc(strlen(wpurl) + 16);
	sprintf(login_url, "%s/wp-login.php", wpurl);
	DEBUG("Built login URL == '%s'.\n", login_url);

	return login_url;
}

static char *wordpress_connection_build_login_body(const char *username,
						   const char *password)
{
	char *username_encoded;
	char *password_encoded;
	char *request_body;

	username_encoded = urlencode(username);
	password_encoded = urlencode(password);

	request_body = malloc(32 + strlen(username_encoded)
				 + strlen(password_encoded));

	sprintf(request_body, "log=%s&pwd=%s&redirect_to=wp-admin",
		username_encoded, password_encoded);
	DEBUG("Built login request == '%s'.\n", request_body);

	/* Zeroize local copy of the password */
	memset(password_encoded, 0, strlen(password_encoded));
	free(password_encoded);
	free(username_encoded);

	return request_body;
}

static void
wordpress_connection_match_logout_url(struct wordpress_connection *connection,
				      struct http_response *response)
{
	unsigned char *body = http_response_get_body(response);

	char *ptr = strstr((const char *) body, "wp-login.php?action=logout");
	if (ptr) {
		char *end = strstr((const char *) ptr, "\"");
		char *wpnonce = malloc(11);
		strncpy(wpnonce, end - 10, 10);

		char *logout_url = malloc(strlen(connection->wpurl) + 48);
		sprintf(logout_url, "%s/wp-login.php?action=logout&_wpnonce=%s",
			connection->wpurl, wpnonce);

		connection->logout_url = logout_url;
	}
}

struct wordpress_connection *
wordpress_connection_initialize(struct options *options)
{
	struct wordpress_connection *connection;

	connection = malloc(sizeof(struct wordpress_connection));
	connection->http_client = http_client_new();
	connection->wpurl = strdup(options->wpurl);
	connection->logout_url = NULL;

	if (options->ignore_ssl_errors) {
		http_client_skip_ssl_validation(connection->http_client);
	}

	return connection;
}

void wordpress_connection_free(struct wordpress_connection *connection)
{
	http_client_free(connection->http_client);
	if (connection->logout_url) {
		free(connection->logout_url);
	}
	free(connection->wpurl);
	free(connection);
}

bool wordpress_connection_login(struct wordpress_connection *connection,
				const char *username, const char *password)
{
	struct http_request *request;
	struct http_response *response;
	char *login_url;
	char *request_body;
	bool retval;

	login_url = wordpress_connection_build_login_url(connection->wpurl);
	request_body = wordpress_connection_build_login_body(username, password);

	request = http_request_new(login_url);
	http_request_set_method(request, HTTP_METHOD_POST);
	http_request_set_body(request, request_body,
			      "application/x-www-form-urlencoded");


	response = http_client_send(connection->http_client, request);
	DEBUG("Request sent, status code is %d.\n",
	      http_response_get_code(response));
	http_request_free(request);

	wordpress_connection_match_logout_url(connection, response);
	if (connection->logout_url) {
		retval = true;
	} else {
		retval = false;
	}

	http_response_free(response);

	/* Zeroize password in the body of the request */
	memset(request_body, 0, strlen(request_body));
	free(request_body);
	free(login_url);

	return retval;
}

bool wordpress_connection_logout(struct wordpress_connection *connection)
{
	bool retval = false;

	if (connection->logout_url) {
		struct http_request *request;
		struct http_response *response;

		request = http_request_new(connection->logout_url);
		response = http_client_send(connection->http_client, request);
		DEBUG("Request sent, status code is %d.\n",
		      http_response_get_code(response));

		unsigned char *body = http_response_get_body(response);

		/* DIRTY */
		char *ptr = strstr((const char *) body, "loginform");
		if (ptr) {
			retval = true;
		}

		http_request_free(request);
		http_response_free(response);
	}

	return retval;
}

void
wordpress_connection_download_to_stream(struct wordpress_connection *connection,
					char *url, FILE *stream)
{
	struct http_request *request;
	struct http_response *response;

	request = http_request_new(url);
	response = http_client_send(connection->http_client, request);
	DEBUG("Request sent, status code is %d.\n",
	      http_response_get_code(response));
	http_request_free(request);

	unsigned char *body = http_response_get_body(response);
	fwrite(body, 1, http_response_get_body_length(response), stream);

	http_response_free(response);
}

void
wordpress_connection_download_to_file(struct wordpress_connection *connection,
				      char *url, const char *filename)
{
	FILE *stream;
	stream = fopen(filename, "w");
	wordpress_connection_download_to_stream(connection, url, stream);
	fclose(stream);
}
