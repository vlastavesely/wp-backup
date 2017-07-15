#include <stdlib.h>
#include <string.h>
#include <libsoup/soup.h>

#include <wpbackup.h>

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

	struct http_response *response = http_response_new(code, (char *)
						message->response_body->data);
	g_object_unref(message);

	return response;
}
