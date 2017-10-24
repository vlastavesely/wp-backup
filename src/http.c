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
#include <libsoup/soup.h>
#include <wp-backup.h>

struct http_client {
	SoupCookieJar *cookies;
	SoupSession *session;
};

struct http_client *http_client_new(void)
{
	struct http_client *client;

	client = malloc(sizeof(*client));
	client->session = soup_session_new();
	client->cookies = soup_cookie_jar_new();
	soup_session_add_feature(client->session, SOUP_SESSION_FEATURE(client->cookies));
	return client;
}

void http_client_free(struct http_client *client)
{
	g_object_unref(client->session);
	g_object_unref(client->cookies);
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
	return response;
}

void http_response_free(struct http_response *response)
{
	if (response->body)
		free(response->body);
	free(response);
}

static SoupMessage *http_client_build_soup_message(struct http_request *request)
{
	SoupMessage *message;

	assert(request->url != NULL);

	if ((message = soup_message_new(request->method, request->url)) == NULL)
		fatal("failed to parse URL '%s'.", request->url);
	if (request->body)
		soup_message_set_request(message, "application/x-www-form-urlencoded",
			SOUP_MEMORY_COPY, request->body, strlen(request->body));
	return message;
}

static int http_client_send_soup_message(struct http_client *client,
		 SoupMessage *message)
{
	int code;

	DEBUG("Sending a request to '%s'...\n",
		soup_uri_to_string(soup_message_get_uri(message), FALSE));
	code = soup_session_send_message(client->session, message);
	DEBUG("Request sent, status code is %d.\n", code);
	return code;
}

struct http_response *http_client_send(struct http_client *client,
		struct http_request *request)
{
	SoupMessage *message = http_client_build_soup_message(request);
	struct http_response *response;
	SoupBuffer *buffer;

	response =http_response_new();
	response->code = http_client_send_soup_message(client, message);

	buffer = soup_message_body_flatten(message->response_body);
	soup_buffer_get_data(buffer, (const unsigned char **) &(response->body),
			     &(response->length));
	soup_buffer_free(buffer);
	return response;
}

static void http_client_got_chunk(SoupMessage *message, SoupBuffer *chunk,
		void *data)
{
	const unsigned char *bytes = NULL;
	size_t sz = 0;
	FILE *fp = (FILE *) data;

	soup_buffer_get_data(chunk, &bytes, &sz);
	fwrite(bytes, sz, 1, fp);
}

int http_client_download_file(struct http_client *client,
		struct http_request *request,
		const char *filename)
{
	SoupMessage *message = http_client_build_soup_message(request);
	int code;
	FILE *fp;

	if ((fp = fopen(filename, "w")) == NULL)
		fatal("failed to open file '%s' for writing.", filename);

	/* Do not accumulate downloaded data in the memory. */
	soup_message_set_flags(message, SOUP_MESSAGE_OVERWRITE_CHUNKS);
	g_signal_connect(G_OBJECT(message), "got-chunk",
			 G_CALLBACK(http_client_got_chunk), fp);

	code = http_client_send_soup_message(client, message);
	fclose(fp);

	return code == 200 ? 0 : code;
}
