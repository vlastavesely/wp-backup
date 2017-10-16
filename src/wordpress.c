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
#include <assert.h>

#include <wp-backup.h>

struct wordpress {
	struct http_client *http_client;
	const char *wpurl;
	char *logout_url;
};

static char *wordpress_build_url(const char *wpurl, const char *path)
{
	char *url = malloc(strlen(wpurl) + strlen(path) + 2);
	assert(path[0] == '/');

	strcpy(url, wpurl);
	/*
	 * The URL must not contain two slashes after root of the
	 * wordpress installation! We must generate proper URL with
	 * just one slash inside in order to prevent unexpected fail
	 * of exporting a dump.
	 */
	if (url[strlen(wpurl) - 1] == '/')
		url[strlen(wpurl) - 1] = '\0';
	strcat(url, path);
	return url;
}

static char *wordpress_build_login_body(const char *username,
					const char *password)
{
	char *body = malloc(32 + (strlen(username) * 3)
			       + (strlen(password) * 3) + 1);
	strcpy(body, "log=");
	urlencode_to_buf(username, body + strlen(body));
	strcat(body, "&pwd=");
	urlencode_to_buf(password, body + strlen(body));
	strcat(body, "&redirect_to=wp-admin");
	return body;
}

static void wordpress_match_logout_url(struct wordpress *connection,
				       struct http_response *response)
{
	unsigned char *body = http_response_get_body(response);
	const char *ptr = NULL;
	char *path;

	ptr = strstr((const char *) body, "/wp-login.php?action=logout");
	if (ptr) {
		*(strstr(ptr, "\"")) = 0;

		/*
		 * We do need match the `wpnonce` token here and build
		 * the logout URL on our own since we cannot rely on its
		 * format. It may contain escaped ampersand or another
		 * entities and thus, it may not be a valid URL without
		 * decoding or other fixes.
		 */
		path = malloc(50);
		sprintf(path, "/wp-login.php?action=logout&_wpnonce=%s",
			strstr(ptr, "_wpnonce") + 9);
		connection->logout_url = wordpress_build_url(connection->wpurl, path);
		free(path);
	}
	free(body);
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
	char *url;
	char *request_body;
	int retval;

	url = wordpress_build_url(connection->wpurl, "/wp-login.php");
	request_body = wordpress_build_login_body(username, password);

	request = http_request_new(url);
	http_request_set_method(request, HTTP_METHOD_POST);
	http_request_set_body(request, request_body);

	response = http_client_send(connection->http_client, request);
	http_request_free(request);

	wordpress_match_logout_url(connection, response);
	retval = connection->logout_url ? 0 : 1;

	http_response_free(response);

	/* Zeroize password in the body of the request */
	memset(request_body, 0, strlen(request_body));
	free(request_body);
	free(url);

	return retval;
}

int wordpress_export(struct wordpress *connection, const char *filename)
{
	char *url;
	int ret;

	url = wordpress_build_url(connection->wpurl,
			"/wp-admin/export.php?content=all&download=true");
	ret = wordpress_download_to_file(connection, url, filename);
	free(url);
	return ret;
}

int wordpress_logout(struct wordpress *connection)
{
	struct http_request *request;
	struct http_response *response;
	unsigned char *body;
	char *ptr;

	if (!connection->logout_url)
		return 1;

	request = http_request_new(connection->logout_url);
	response = http_client_send(connection->http_client, request);

	body = http_response_get_body(response);

	/* DIRTY */
	ptr = strstr((const char *) body, "loginform");

	http_request_free(request);
	http_response_free(response);

	return ptr ? 0 : 1;
}

int wordpress_download_to_file(struct wordpress *connection, char *url,
			       const char *filename)
{
	struct http_request *request;
	struct http_response *response;
	int code;

	request = http_request_new(url);
	http_request_set_filename(request, (char *) filename);
	response = http_client_send(connection->http_client, request);
	code = http_response_get_code(response);
	http_request_free(request);
	http_response_free(response);
	return code == 200 ? 0 : code;
}
