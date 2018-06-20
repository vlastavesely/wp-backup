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
#include "http.h"
#include "err.h"
#include "utils.h"

#include <curl/curl.h>

static const char *cookie_jar_template = "/tmp/wp-backup-cookies-XXXXXX";

/*
 * Structure for session data storage. It holds data we need between
 * multiple HTTP requests sent.
 */
struct http_client {
	char *cookiejar;
	CURL *curl;
};

static CURL *initialize_curl(const char *cookiejar)
{
	CURL *curl;

	curl = curl_easy_init();
	if (!curl)
		return ERR_PTR(-1);

	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

//	/* Read cookies from this file. */
//	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiejar);

//	/* Save cookies to this file after `curl_easy_cleanup()` called. */
//	curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiejar);

	return curl;
}

struct http_client *alloc_http_client(void)
{
	struct http_client *client;
	char *jarname;
	CURL *curl;

	jarname = mktemp_filename(cookie_jar_template);
	if (IS_ERR(jarname))
		return ERR_CAST(jarname);

	curl = initialize_curl(jarname);
	if (IS_ERR(curl)) {
		client = ERR_CAST(curl);
		goto drop_jarname;
	}

	client = malloc(sizeof(*client));
	if (client == NULL) {
		client = ERR_PTR(-ENOMEM);
		goto drop_curl;
	}

	client->cookiejar = jarname;
	client->curl = curl;

out:
	return client;
drop_curl:
	curl_easy_cleanup(curl);
drop_jarname:
	free(jarname);
	goto out;
}

void drop_http_client(struct http_client *client)
{
	if (IS_ERR_OR_NULL(client))
		return;

	if (zeroize_file(client->cookiejar) != 0)
		error("failed to zeroize cookie jar.");
	if (unlink(client->cookiejar) != 0)
		error("failed to remove cookie jar.");

	free(client->cookiejar);
	curl_easy_cleanup(client->curl);

	free(client);
}

/******************************************************************************/

struct http_request *alloc_http_request()
{
	struct http_request *request;

	request = malloc(sizeof(*request));
	if (!request)
		return ERR_PTR(-ENOMEM);

	memset(request, 0, sizeof(*request));
	request->method = "GET";
	return request;
}

void drop_http_request(struct http_request *request)
{
	if (IS_ERR_OR_NULL(request))
		return;

	free(request->url);
	free(request->body);
	free(request);
}

/******************************************************************************/

static struct http_response *alloc_http_response()
{
	struct http_response *response;

	response = malloc(sizeof(*response));
	if (!response)
		return ERR_PTR(-ENOMEM);

	memset(response, 0, sizeof(*response));
	return response;
}

void drop_http_response(struct http_response *response)
{
	if (IS_ERR_OR_NULL(response))
		return;

	free(response->body);
	free(response->content_type);
	free(response);
}

/******************************************************************************/

struct string_buffer {
	char *data;
	size_t nbytes;
};

static void string_buffer_init(struct string_buffer *str)
{
	str->data = NULL;
	str->nbytes = 0;
}

static void string_buffer_clear(struct string_buffer *str)
{
	free(str->data);
	str->data = NULL;
	str->nbytes = 0;
}

/*
 * Write callback for cURL. On success, it should return number of bytes
 * that were actually written. If return value differs, the call is
 * considered to be failed.
 */
static size_t str_buffer_append(void *ptr, size_t size, size_t nmemb,
	struct string_buffer *str)
{
	size_t length = str->nbytes + (size * nmemb);

	str->data = realloc(str->data, length + 1);
	if (!str->data)
		return 0;

	memcpy(str->data + str->nbytes, ptr, size * nmemb);
	str->data[length] = '\0';
	str->nbytes = length;

	return size * nmemb;
}

/******************************************************************************/

static struct http_response *http_curl_perform(CURL *curl)
{
	struct http_response *response;
	unsigned int code = 0;
	const char *mime = NULL;
	int retval;

	retval = curl_easy_perform(curl);
	if (retval != 0)
		return ERR_PTR(-retval);

	retval = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	if (retval != 0)
		return ERR_PTR(-retval);

	retval = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &mime);
	if (retval != 0)
		return ERR_PTR(-retval);

	response = alloc_http_response();
	response->code = code;
	response->content_type = strdup(mime);

	return response;
}

/*
 * Sends a request to a server and returns body of the response.
 */
struct http_response *http_client_send(struct http_client *client,
		struct http_request *request)
{
	struct http_response *response;
	struct string_buffer str;
	CURL *curl = client->curl;

	string_buffer_init(&str);
	curl_easy_setopt(curl, CURLOPT_URL, request->url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, str_buffer_append);

	if (strcmp(request->method, "POST") == 0) {
		/* POST request */
		assert(request->body != NULL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request->body));
	}

	response = http_curl_perform(curl);
	if (IS_ERR(response))
		goto err;

	/*
	 * Response takes over the string, it won't be freed here
	 * but with freeing of the response.
	 */
	response->body = str.data;

out:
	return response;
err:
	string_buffer_clear(&str);
	goto out;
}

/*
 * Sends a request to a server and saves downloaded data into a file.
 * Returns a HTTP response with metadata and empty body.
 */
struct http_response *http_client_download_file(struct http_client *client,
		struct http_request *request,
		const char *filename)
{
	struct http_response *response;
	CURL *curl = client->curl;

	FILE *fp;

	fp = fopen(filename, "w");
	if (fp == NULL)
		return ERR_PTR(-errno);

	curl_easy_setopt(curl, CURLOPT_URL, request->url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);

	response = http_curl_perform(curl);
	fclose(fp);

	return response;
}
