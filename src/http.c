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


struct http_client {
	CURL *curl;
};

struct http_client *http_client_alloc(void)
{
	struct http_client *client;
	CURL *curl;

	curl = curl_easy_init();
	if (IS_ERR(curl))
		return ERR_CAST(curl);

	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	/* curl_easy_setopt(curl, CURLOPT_VERBOSE, 1); */

	client = malloc(sizeof(*client));
	if (client == NULL) {
		curl_easy_cleanup(curl);
		return ERR_PTR(-ENOMEM);
	}

	client->curl = curl;

	return client;
}

void http_client_drop(struct http_client *client)
{
	if (IS_ERR_OR_NULL(client))
		return;

	curl_easy_cleanup(client->curl);
	free(client);
}

/******************************************************************************/

static struct http_response *http_response_alloc()
{
	struct http_response *response;

	response = malloc(sizeof(*response));
	if (!response)
		return ERR_PTR(-ENOMEM);

	memset(response, 0, sizeof(*response));

	return response;
}

void http_response_drop(struct http_response *response)
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

static struct http_response *http_curl_get_response(CURL *curl)
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

	response = http_response_alloc();
	response->code = code;
	response->content_type = strdup(mime);

	return response;
}

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
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request->method);

	if (strcmp(request->method, "POST") == 0) {
		assert(request->body != NULL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request->body));
	}

	response = http_curl_get_response(curl);
	if (IS_ERR(response))
		goto err;

	/* Response takes over the string, it won't be freed here
	 * but with freeing of the response. */
	response->body = str.data;

out:
	return response;
err:
	string_buffer_clear(&str);
	goto out;
}

int http_client_download_file(struct http_client *client,
			      struct http_request *request,
			      const char *filename)
{
	CURL *curl = client->curl;
	int retval;
	FILE *fp;

	fp = fopen(filename, "w");
	if (fp == NULL)
		return -errno;

	curl_easy_setopt(curl, CURLOPT_URL, request->url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);

	retval = curl_easy_perform(curl);
	fclose(fp);

	return -retval;
}
