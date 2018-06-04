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

#include <curl/curl.h>

/*
 * Structure for session data storage. It holds data we need between
 * multiple HTTP requests sent.
 */
struct http_client {
	char *cookiejar;
};

struct http_client *alloc_http_client(void)
{
	struct http_client *client;

	client = malloc(sizeof(*client));
	if (!client)
		return ERR_PTR(-ENOMEM);

	/* TODO: Generate random filename */
	client->cookiejar = strdup("/tmp/wp-backup-cookies.txt");
	return client;
}

static int zeroize_file(const char *filename)
{
	struct stat st;
	void *map;
	size_t len;
	int fd, retval = 0;

	if (stat(filename, &st) != 0) {
		retval = -errno;
		goto out;
	}

	fd = open(filename, O_RDWR);
	if (fd == -1) {
		retval = -errno;
		goto out;
	}

	len = st.st_size;
	map = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		retval = -errno;
		goto out;
	}

	memset(map, 0, len);

	munmap(map, len);
	close(fd);

out:
	return retval;
}

void drop_http_client(struct http_client *client)
{
	if (IS_ERR_OR_NULL(client))
		return;

	if (client->cookiejar) {
		zeroize_file(client->cookiejar);
		unlink(client->cookiejar);
		free(client->cookiejar);
	}
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

	if (request->url)
		free(request->url);
	if (request->body)
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

	if (response->body)
		free(response->body);
	if (response->content_type)
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

/*
 * Initializes default cURL context from request data.
 */
static CURL *build_curl_instance(struct http_client *client,
		struct http_request *request)
{
	CURL *curl;

	curl = curl_easy_init();
	if (!curl)
		return ERR_PTR(-1);

	curl_easy_setopt(curl, CURLOPT_URL, request->url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

	/* Read cookies from this file. */
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, client->cookiejar);

	/* Save cookies to this file after `curl_easy_cleanup()` called. */
	curl_easy_setopt(curl, CURLOPT_COOKIEJAR, client->cookiejar);

	if (strcmp(request->method, "GET") == 0)
		goto out;

	/* POST request */
	assert(request->body != NULL);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request->body));

out:
	return curl;
}

static struct http_response *http_curl_perform(CURL *curl)
{
	struct http_response *response;
	unsigned int http_code = 0;
	const char *content_type = NULL;

	if (curl_easy_perform(curl) != CURLE_OK)
		return ERR_PTR(-1);

	if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code) != 0)
		return ERR_PTR(-2);

	if (curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type) != 0)
		return ERR_PTR(-3);

	response = alloc_http_response();
	response->code = http_code;
	response->content_type = strdup(content_type);
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
	CURL *curl;

	string_buffer_init(&str);
	curl = build_curl_instance(client, request);
	if (IS_ERR(curl))
		return (struct http_response *) curl; /* Error code */

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, str_buffer_append);
	response = http_curl_perform(curl);
	if (IS_ERR(response))
		goto out;

	/*
	 * Response takes over the string, it won't be freed here
	 * but with freeing of the response.
	 */
	response->body = str.data;

out:
	curl_easy_cleanup(curl);
	return response;
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
	int err;
	CURL *curl;
	FILE *fp;

	fp = fopen(filename, "w");
	if (!fp)
		return ERR_PTR(-errno);

	curl = build_curl_instance(client, request);
	if (IS_ERR(curl))
		return (struct http_response *) curl; /* Error code */

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

	response = http_curl_perform(curl);

	fclose(fp);
	curl_easy_cleanup(curl);
	return response;
}
