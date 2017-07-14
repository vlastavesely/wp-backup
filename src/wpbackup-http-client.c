#include <stdlib.h>
#include <string.h>
#include <libsoup/soup.h>

#include <wpbackup.h>

struct wpbackup_http_client
{
	SoupCookieJar *cookies;
	SoupSession *session;
};

struct wpbackup_http_client *wpbackup_http_client_new()
{
	struct wpbackup_http_client *client
		= malloc(sizeof(struct wpbackup_http_client));

	client->session = soup_session_new();
	client->cookies = soup_cookie_jar_new();
	soup_session_add_feature(client->session,
				 SOUP_SESSION_FEATURE(client->cookies));
	return client;
}

void wpbackup_http_client_free(struct wpbackup_http_client *client)
{
	g_object_unref(client->session);
	g_object_unref(client->cookies);
	free(client);
}


static char *
wpbackup_http_client_stringify_method(struct wpbackup_http_request *request)
{
	if (wpbackup_http_request_get_method(request) == WPBACKUP_HTTP_METHOD_POST) {
		return strdup("POST");
	} else {
		return strdup("GET");
	}
}

static void
wpbackup_http_client_set_body_into_message(SoupMessage *message,
					   struct wpbackup_http_request *request)
{
	if (wpbackup_http_request_get_body(request)) {
		char *body = wpbackup_http_request_get_body(request);
		soup_message_set_request(message,
			wpbackup_http_request_get_content_type(request),
			SOUP_MEMORY_COPY, body, strlen(body));
		free(body);
	}
}

struct wpbackup_http_response *
wpbackup_http_client_send(struct wpbackup_http_client *client,
			  struct wpbackup_http_request *request)
{
	SoupMessage *message;
	unsigned int code;
	char *method;

	method = wpbackup_http_client_stringify_method(request);
	message = soup_message_new(method, wpbackup_http_request_get_url(request));
	free(method);

	wpbackup_http_client_set_body_into_message(message, request);

	DEBUG("Sending a request to '%s'...\n", wpbackup_http_request_get_url(request));
	code = soup_session_send_message(client->session, message);

	struct wpbackup_http_response *response
		= wpbackup_http_response_new(code, (char *)
					     message->response_body->data);

	g_object_unref(message);

	return response;
}
