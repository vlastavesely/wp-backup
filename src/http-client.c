#include <stdlib.h>
#include <string.h>
#include <libsoup/soup.h>

#include <wp-backup/debug.h>
#include <wp-backup/http-client.h>

struct http_client
{
	SoupCookieJar *cookies;
	SoupSession *session;
};

struct http_client *http_client_new()
{
	struct http_client *client = malloc(sizeof(struct http_client));

	client->session = soup_session_new();
	client->cookies = soup_cookie_jar_new();
	soup_session_add_feature(client->session,
				 SOUP_SESSION_FEATURE(client->cookies));
	return client;
}

void http_client_free(struct http_client *client)
{
	g_object_unref(client->session);
	g_object_unref(client->cookies);
	free(client);
}


static char *http_client_stringify_method(struct http_request *request)
{
	if (http_request_get_method(request) == HTTP_METHOD_POST) {
		return strdup("POST");
	} else {
		return strdup("GET");
	}
}

static void http_client_set_body_into_message(SoupMessage *message,
					      struct http_request *request)
{
	if (http_request_get_body(request)) {
		char *body = http_request_get_body(request);
		soup_message_set_request(message,
			http_request_get_content_type(request),
			SOUP_MEMORY_COPY, body, strlen(body));
		free(body);
	}
}

struct http_response *http_client_send(struct http_client *client,
				       struct http_request *request)
{
	SoupMessage *message;
	unsigned int code;
	char *method;

	method = http_client_stringify_method(request);
	message = soup_message_new(method, http_request_get_url(request));
	free(method);

	http_client_set_body_into_message(message, request);

	DEBUG("Sending a request to '%s'...\n", http_request_get_url(request));
	code = soup_session_send_message(client->session, message);

	/* TODO - it would be nice to allow using some callback saving
	 * chunks directly instead of storing all the chunks in the
	 * memory. It could be a problem for huge files. */
	SoupBuffer *buffer;
	unsigned char *data;
	size_t length;

	/* Gets length and pointer to WHOLE response's body */
	buffer = soup_message_body_flatten(message->response_body);
	soup_buffer_get_data(buffer, (const guint8 **) &data, &length);

	struct http_response *response = http_response_new(code, data, length);
	g_object_unref(message);
	soup_buffer_free(buffer);

	return response;
}
