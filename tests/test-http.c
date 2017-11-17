/*
 * Copyright (C) 2017 Vlasta Vesely <vlastavesely@protonmail.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <CUnit/CUnit.h>

#include "../src/http.h"

static void test_http_request(void)
{
	struct http_request *request;

	request = http_request_new();
	CU_ASSERT_STRING_EQUAL("GET", request->method);
	CU_ASSERT_PTR_NULL(request->url);
	CU_ASSERT_PTR_NULL(request->body);

	request->url = strdup("http://example.com/");
	request->method = "POST"; /* const char */
	request->body = strdup("user=admin&pwd=secret");

	http_request_free(request);
}

static void test_http_request_send(void)
{
	struct http_client *client;
	struct http_request *request;
	struct http_response *response;

	client = http_client_new();

	request = http_request_new();
	request->url = strdup("http://localhost");

	response = http_client_send(client, request);
	CU_ASSERT_EQUAL(200, response->code);

	http_client_free(client);
	http_request_free(request);
	http_response_free(response);
}

void test_http_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_http_request",
		test_http_request);
	CU_add_test(suite, "test_http_request_send",
		test_http_request_send);
}
