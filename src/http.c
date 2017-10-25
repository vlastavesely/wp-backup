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
#include <unistd.h>
#include <assert.h>
#include <curl/curl.h>

#include "http.h"
#include "debug.h"
#include "error.h"

/*
 * Structure for session data storage. It holds data we need between
 * multiple HTTP requests sent.
 */
struct http_client {
	char *cookiejar;
};

struct string_buffer {
	char *data;
	size_t nbytes;
};

struct http_client *http_client_new(void)
{
	struct http_client *client;

	client = malloc(sizeof(*client));
	/*
	 * TODO: Generate random filename
	 */
	client->cookiejar = strdup("/tmp/wp-backup-cookies.txt");

	return client;
}

void http_client_free(struct http_client *client)
{
	if (client->cookiejar) {
		unlink(client->cookiejar);
		free(client->cookiejar);
	}
	free(client);
}

struct http_request *http_request_new()
{
	struct http_request *request;

	request = malloc(sizeof(*request));
	request->method = "GET";
	request->url = NULL;
	request->body = NULL;
	return request;
}

void http_request_free(struct http_request *request)
{
	if (request->url)
		free(request->url);
	if (request->body)
		free(request->body);
	free(request);
}

static struct http_response *http_response_new()
{
	struct http_response *response;

	response = malloc(sizeof(*response));
	response->code = 0;
	response->body = NULL;
	response->length = 0;
	response->content_type = NULL;
	return response;
}

void http_response_free(struct http_response *response)
{
	if (response->body)
		free(response->body);
	if (response->content_type)
		free(response->content_type);
	free(response);
}

static void string_buffer_init(struct string_buffer *str)
{
	str->data = NULL;
	str->nbytes = 0;
}

static size_t str_buffer_append(void *ptr, size_t size, size_t nmemb,
	struct string_buffer *str)
{
	size_t length = str->nbytes + (size * nmemb);

	str->data = realloc(str->data, length + 1);
	if (!str->data)
		fatal("failed to realloc() string buffer.");

	memcpy(str->data + str->nbytes, ptr, size * nmemb);
	str->data[length] = '\0';
	str->nbytes = length;

	return size * nmemb;
}

/*
 * Initializes default cURL context from request data.
 */
static CURL *http_curl_new(struct http_client *client,
		struct http_request *request)
{
	CURL *curl;

	if ((curl = curl_easy_init()) == NULL)
		fatal("failed to initialize cURL context.");

	curl_easy_setopt(curl, CURLOPT_URL, request->url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

	/* Read cookies from this file. */
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, client->cookiejar);

	/* Save cookies to this file after `curl_easy_cleanup()` called. */
	curl_easy_setopt(curl, CURLOPT_COOKIEJAR, client->cookiejar);

	if (strcmp(request->method, "POST") == 0) {
		assert(request->body != NULL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request->body));
	}

	return curl;
}

static struct http_response *http_curl_perform(CURL *curl)
{
	struct http_response *response;
	unsigned int http_code = 0;
	const char *content_type = NULL;

	if (curl_easy_perform(curl) == CURLE_HTTP_RETURNED_ERROR)
		fatal("failed to get response from the server.");

	if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code) != 0)
		fatal("failed to fetch HTTP status code from response.");

	if (curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type) != 0)
		fatal("failed to fetch content-type from response.");

	response = http_response_new();
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
	curl = http_curl_new(client, request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, str_buffer_append);

	DEBUG("Sending a request to '%s'...\n", request->url);
	response = http_curl_perform(curl);
	DEBUG("Request sent, status code is %d.\n", response->code);

	/*
	 * Response takes over the string, it won't be freed here
	 * but with freeing of the response.
	 */
	response->body = str.data;

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
	CURL *curl;
	FILE *fp;

	if ((fp = fopen(filename, "w")) == NULL)
		fatal("failed to open file '%s' for writing.", filename);

	curl = http_curl_new(client, request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

	DEBUG("Downloading file from '%s'...\n", request->url);
	response = http_curl_perform(curl);
	DEBUG("File download completed, status code is %d.\n", response->code);

	fclose(fp);
	curl_easy_cleanup(curl);
	return response;
}
