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
#include <errno.h>

#include "wordpress.h"
#include "http.h"
#include "wxr-feed.h"
#include "utils.h"
#include "err.h"

/*
 * This structure represents a connection to a remote WordPress
 * installation. It contains pointer to a HTTP client holding
 * information about a session identifier to hold login session
 * open. If user has been logged in successfully, property
 * 'logout_url' is set to URL for logout.
 */
struct wordpress {
	struct http_client *http_client;
	const char *wpurl;
	char *logout_url;
};

/*
 * Builds an absolute URL from a relative one by proper appending
 * of path to root path of the WordPress installation.
 */
static char *wordpress_build_url(const char *wpurl, const char *path)
{
	char *url;

	assert(path[0] == '/');

	if ((url = malloc(strlen(wpurl) + strlen(path) + 2)) == NULL)
		return ERR_PTR(-ENOMEM);

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

/*
 * Builds a body of a POST request for login into WordPress administration
 * containing username and password. It fakes standard login through
 * a login page.
 */
static char *wordpress_build_login_body(const char *username,
					const char *password)
{
	unsigned int len;
	char *body;

	/* Allocates a buffer for encoded URL. Since any character could be
	 * expanded into '%XX' form, the buffer must be three tines bigger
	 * than all strings that will be decoded. */
	len = 32 + (strlen(username) * 3) + (strlen(password) * 3) + 1;
	if ((body = malloc(len)) == NULL)
		return ERR_PTR(-ENOMEM);

	strcpy(body, "log=");
	urlencode_to_buf(username, body + strlen(body));
	strcat(body, "&pwd=");
	urlencode_to_buf(password, body + strlen(body));
	strcat(body, "&redirect_to=wp-admin");

	return body;
}

/*
 * Looks for logout URL in body of a HTTP response. If found, it means
 * that our login action can be considered to be successful.
 */
static void wordpress_match_logout_url(struct wordpress *connection,
				       struct http_response *response)
{
	const char *ptr = NULL;
	char path[128];

	if ((ptr = strstr(response->body, "/wp-login.php?action=logout"))) {
		*(strstr(ptr, "\"")) = 0;
		/*
		 * Logout URL does contain characters '&' (maybe even other
		 * characters) encoded as HTML entity so we cannot use it
		 * without proper decoding: logout would fail without it.
		 */
		html_decode_entities_to_buf(ptr, path);
		connection->logout_url = wordpress_build_url(connection->wpurl, path);
	}
}

struct wordpress *wordpress_create(const char *wpurl)
{
	struct wordpress *connection;

	connection = malloc(sizeof(*connection));
	connection->http_client = http_client_new();
	connection->wpurl = wpurl;
	connection->logout_url = NULL;
	/* if (options->ignore_ssl_errors)
	 *	http_client_skip_ssl_validation(connection->http_client); */

	return connection;
}

void wordpress_free(struct wordpress *connection)
{
	if (connection == NULL)
		return;

	http_client_free(connection->http_client);
	if (connection->logout_url)
		free(connection->logout_url);
	free(connection);
}

/*
 * Performs a login. If body of the HTTP response after login does
 * contain a valid logout URL, it the login is considered to be
 * successful and zero code is returned, -1 otherwise.
 */
int wordpress_login(struct wordpress *connection, const char *username,
		const char *password)
{
	struct http_request *request;
	struct http_response *response;
	char *url;
	int retval;

	url = wordpress_build_url(connection->wpurl, "/wp-login.php");
	if (IS_ERR(url)) {
		retval = PTR_ERR(url);
		goto out;
	}

	request = http_request_new();
	if (IS_ERR(request)) {
		retval = PTR_ERR(request);
		goto out;
	}

	request->method = "POST";
	request->url = url;
	request->body = wordpress_build_login_body(username, password);

	response = http_client_send(connection->http_client, request);
	wordpress_match_logout_url(connection, response);
	retval = connection->logout_url ? 0 : -1;

	/* Zeroize password in the body of the request */
	memset(request->body, 0, strlen(request->body));
	http_request_free(request);
	http_response_free(response);

out:
	return retval;
}

/*
 * Saves an export dump to a file. To be considered as success, the response
 * must be of 'text/xml' or 'application/xml' and contain valid XML data.
 */
int wordpress_export(struct wordpress *connection, const char *filename)
{
	struct http_response *response;
	struct wxr_feed *feed;
	char *url;
	int retval;

	url = wordpress_build_url(connection->wpurl, "/wp-admin/export.php?content=all&download=true");
	response = wordpress_download_to_file(connection, url, filename);

	/*
	 * Tries to load downloaded XML to check its validity.
	 * If the data are corrupted, download failed.
	 */
	feed = wxr_feed_load(filename);
	if (IS_ERR(feed))
		retval = PTR_ERR(feed);

//	if (error) { FIXME
//		fatal(error->message);
//	}
	retval = (feed == NULL || response->code != 200);

	if (feed)
		wxr_feed_free(feed);

	http_response_free(response);
	free(url);
	return retval;
}

/*
 * Invalidates a session by logout.
 */
int wordpress_logout(struct wordpress *connection)
{
	struct http_request *request;
	struct http_response *response;
	const char *ptr = NULL;

	if (connection->logout_url == NULL)
		fatal("failed to logout - logout URL missing.");

	request = http_request_new();
	request->url = strdup(connection->logout_url);
	response = http_client_send(connection->http_client, request);

	/*
	 * If the reponse contains a 'loginform', it can be considered
	 * to be standard login page (so logout has been successful).
	 */
	ptr = strstr((const char *) response->body, "loginform");

	http_request_free(request);
	http_response_free(response);

	return ptr == NULL;
}

/*
 * Donwloads a file from an absolute URL.
 */
struct http_response *wordpress_download_to_file(struct wordpress *connection,
		const char *url, const char *filename)
{
	struct http_request *request;
	struct http_response *response;

	request = http_request_new();
	request->url = strdup(url);
	response = http_client_download_file(connection->http_client, request, filename);
	http_request_free(request);

	return response;
}
