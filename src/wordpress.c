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

struct wordpress {
	struct http_client *http_client;
	const char *wpurl;
	char *logout_url;
};

static char *wordpress_build_login_url(const char *wpurl)
{
	char *url = malloc(strlen(wpurl) + 16);

	sprintf(url, "%s/wp-login.php", wpurl);
	return url;
}

static char *wordpress_build_login_body(const char *username,
					const char *password)
{
	char *username_encoded = urlencode(username);
	char *password_encoded = urlencode(password);
	char *request_body = malloc(32 + strlen(username_encoded)
				       + strlen(password_encoded));

	sprintf(request_body, "log=%s&pwd=%s&redirect_to=wp-admin",
		username_encoded, password_encoded);

	/* Zeroize local copy of the password */
	memset(password_encoded, 0, strlen(password_encoded));
	free(password_encoded);
	free(username_encoded);

	return request_body;
}

static void wordpress_match_logout_url(struct wordpress *connection,
				       struct http_response *response)
{
	unsigned char *body = http_response_get_body(response);
	char *end, *wpnonce, *url;

	char *ptr = strstr((const char *) body, "wp-login.php?action=logout");
	if (ptr) {
		end = strstr((const char *) ptr, "\"");
		wpnonce = malloc(11);
		strncpy(wpnonce, end - 10, 10);

		url = malloc(strlen(connection->wpurl) + 48);
		sprintf(url, "%s/wp-login.php?action=logout&_wpnonce=%s",
			connection->wpurl, wpnonce);

		connection->logout_url = url;
	}
}

struct wordpress *wordpress_create(const char *wpurl)
{
	struct wordpress *connection;

	connection = malloc(sizeof(*connection));

	connection->http_client = http_client_new();
	connection->wpurl = wpurl;
	connection->logout_url = NULL;

//	if (options->ignore_ssl_errors) {
//		http_client_skip_ssl_validation(connection->http_client);
//	}

	return connection;
}

void wordpress_free(struct wordpress *connection)
{
	http_client_free(connection->http_client);
	if (connection->logout_url)
		free(connection->logout_url);
	free(connection);
}

int wordpress_login(struct wordpress *connection, const char *username,
		    const char *password)
{
	struct http_request *request;
	struct http_response *response;
	char *login_url;
	char *request_body;
	int retval;

	login_url = wordpress_build_login_url(connection->wpurl);
	request_body = wordpress_build_login_body(username, password);

	request = http_request_new(login_url);
	http_request_set_method(request, HTTP_METHOD_POST);
	http_request_set_body(request, request_body,
			      "application/x-www-form-urlencoded");


	response = http_client_send(connection->http_client, request);
	http_request_free(request);

	wordpress_match_logout_url(connection, response);
	retval = connection->logout_url ? 0 : 1;

	http_response_free(response);

	/* Zeroize password in the body of the request */
	memset(request_body, 0, strlen(request_body));
	free(request_body);
	free(login_url);

	return retval;
}

int wordpress_logout(struct wordpress *connection)
{
	unsigned char *body;
	char *ptr;

	if (connection->logout_url) {
		struct http_request *request;
		struct http_response *response;

		request = http_request_new(connection->logout_url);
		response = http_client_send(connection->http_client, request);

		body = http_response_get_body(response);

		/* DIRTY */
		ptr = strstr((const char *) body, "loginform");

		http_request_free(request);
		http_response_free(response);

		return ptr ? 0 : 1;
	}

	return 1;
}

int wordpress_export(struct wordpress *connection, const char *filename)
{
	char *url = malloc(strlen(connection->wpurl) + 48);
	int ret;

	strcpy(url, connection->wpurl);
	if (url[strlen(url) - 1] != '/')
		strcat(url, "/");

	strcat(url, "wp-admin/export.php?content=all&download=true");
	ret = wordpress_download_to_file(connection, url, filename);
	free(url);

	return ret;
}

int wordpress_download_to_file(struct wordpress *connection, char *url,
			       const char *filename)
{
	struct http_request *request;
	struct http_response *response;
	FILE *fp = fopen(filename, "w");

	if (fp) {
		request = http_request_new(url);
		response = http_client_send(connection->http_client, request);
		http_request_free(request);

		unsigned char *body = http_response_get_body(response);
		fwrite(body, 1, http_response_get_body_length(response), fp);

		http_response_free(response);
		fclose(fp);
		return 0;
	}

	return 1;
}
