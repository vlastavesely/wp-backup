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

#include "compat.h"
#include "wordpress.h"
#include "http.h"
#include "wxr-feed.h"
#include "utils.h"
#include "err.h"

/*
 * This structure represents a connection to a remote WordPress
 * installation. It contains pointer to a HTTP client holding
 * information about a session identifier to hold login session
 * open. If the user has been logged in successfully, property
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

	url = malloc(strlen(wpurl) + strlen(path) + 2);
	if (url == NULL)
		return ERR_PTR(-ENOMEM);

	strcpy(url, wpurl);
	/*
	 * The URL must not contain two slashes after root of the
	 * wordpress installation! We must generate a proper URL with
	 * just one slash inside in order to prevent an unexpected fail
	 * of exporting a dump.
	 */
	if (url[strlen(wpurl) - 1] == '/')
		url[strlen(wpurl) - 1] = '\0';
	strcat(url, path);

	return url;
}

/*
 * Builds a body of the POST request for login into WordPress administration.
 * The body must contain username and password. It fakes standard login through
 * the login page.
 */
static char *wordpress_build_login_body(const char *username,
					const char *password)
{
	unsigned int len;
	char *body;

	/* Allocates a buffer for encoded URL. Since any character could be
	 * expanded into '%XX' form, the buffer must be three times bigger
	 * than all strings that will be decoded. */
	len = 32 + (strlen(username) * 3) + (strlen(password) * 3) + 1;
	body = malloc(len);
	if (body == NULL)
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
static int wordpress_match_logout_url(struct wordpress *connection,
				      struct http_response *response)
{
	const char *ptr = NULL;
	char path[128];

	ptr = strstr(response->body, "/wp-login.php?action=logout");
	if (ptr) {
		*(strstr(ptr, "\"")) = 0;
		/*
		 * Logout URL does contain characters '&' (maybe even other
		 * characters) encoded as HTML entity so we cannot use it
		 * without proper decoding: logout would fail without it.
		 */
		html_decode_entities_to_buf(ptr, path);
		connection->logout_url = wordpress_build_url(connection->wpurl, path);
	}

	return ptr != NULL ? 0 : -1;
}

struct wordpress *wordpress_create(const char *wpurl)
{
	struct wordpress *connection;

	connection = malloc(sizeof(*connection));
	if (!connection)
		return ERR_PTR(-ENOMEM);

	connection->http_client = http_client_alloc();
	connection->wpurl = wpurl;
	connection->logout_url = NULL;
	/* if (options->ignore_ssl_errors) TODO
	 *	http_client_skip_ssl_validation(connection->http_client); */

	return connection;
}

void wordpress_drop(struct wordpress *connection)
{
	if (IS_ERR_OR_NULL(connection))
		return;

	http_client_drop(connection->http_client);
	free(connection->logout_url);
	free(connection);
}

/*
 * Logs into the WordPress installation. If body of the HTTP response
 * after login does contain a valid logout URL, the login is considered
 * to be successful and zero code is returned, -1 otherwise.
 */
int wordpress_login(struct wordpress *connection, const char *username,
		const char *password)
{
	struct http_request request;
	struct http_response *response;
	char *url, *body;
	int retval = 0;

	url = wordpress_build_url(connection->wpurl, "/wp-login.php");
	if (IS_ERR(url)) {
		retval = PTR_ERR(url);
		goto out;
	}

	body = wordpress_build_login_body(username, password);
	if (IS_ERR(body)) {
		retval = PTR_ERR(body);
		goto drop_url;
	}

	request.method = "POST";
	request.url = url;
	request.body = body;

	response = http_client_send(connection->http_client, &request);
	memset(body, 0, strlen(body));

	if (IS_ERR(response)) {
		retval = PTR_ERR(response);
		goto drop_body;
	}

	retval = wordpress_match_logout_url(connection, response);

drop_body:
	free(body);
drop_url:
	free(url);
out:
	return retval;
}

/*
 * Saves an export dump to a file. To be considered as success, the response
 * must be a valid WXR feed.
 */
int wordpress_export(struct wordpress *connection, const char *filename,
		     bool quiet)
{
	struct wxr_feed *feed;
	struct post *walk;
	char *url;
	int retval = 0, posts = 0, pages = 0;

	url = wordpress_build_url(connection->wpurl, "/wp-admin/export.php?content=all&download=true");
	if (IS_ERR(url))
		return PTR_ERR(url);

	retval = wordpress_download_to_file(connection, url, filename);
	free(url);
	if (retval != 0)
		return retval;

	/*
	 * Tries to load the downloaded XML to check its validity.
	 * If the data are corrupted, download failed.
	 */
	feed = wxr_feed_load(filename);
	if (IS_ERR(feed))
		return PTR_ERR(feed);

	if (!quiet) {
		walk = wxr_feed_get_posts(feed);
		while (walk) {
			posts++;
			walk = walk->next;
		}

		walk = wxr_feed_get_pages(feed);
		while (walk) {
			pages++;
			walk = walk->next;
		}

		fprintf(stderr, "Backed up %d post%s and %d page%s.\n", posts,
			posts != 1 ? "s" : "", pages, pages != 1 ? "s" : "");
	}

	wxr_feed_drop(feed);

	return 0;
}

/*
 * Invalidates a session by logout.
 */
int wordpress_logout(struct wordpress *connection)
{
	struct http_request request;
	struct http_response *response;
	const char *ptr = NULL;

	if (connection->logout_url == NULL)
		return -1;

	request.method = "GET";
	request.url = connection->logout_url;

	response = http_client_send(connection->http_client, &request);
	if (!IS_ERR(response)) {
		/*
		 * If the reponse contains a 'loginform', it can be considered
		 * to be standard login page (so logout has been successful).
		 *
		 * TODO: figure out something smarter...
		 */
		ptr = strstr(response->body, "loginform");
		http_response_drop(response);
	}

	return ptr != NULL ? 0 : -2;
}

/*
 * Donwloads a file from an absolute URL.
 */
int wordpress_download_to_file(struct wordpress *connection, const char *url,
			       const char *filename)
{
	struct http_request request;
	int retval;

	request.method = "GET";
	request.url = url;
	retval = http_client_download_file(connection->http_client, &request,
					   filename);
	return retval;
}
