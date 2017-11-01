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

void test_http_add_tests(struct CU_Suite *suite)
{
	CU_add_test(suite, "test_http_request",
		test_http_request);
}
